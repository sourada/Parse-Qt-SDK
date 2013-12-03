//
//  PFQuery.cpp
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFConversion.h"
#include "PFError.h"
#include "PFManager.h"
#include "PFObject.h"
#include "PFQuery.h"
#include "PFUser.h"

// Qt headers
#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>

namespace parse {

#define PFUSER_QUERY_CLASSNAME		"__PFUSER_QUERY__"

#pragma mark - Memory Management Methods

PFQuery::PFQuery()
{
	qDebug().nospace() << "Created PFQuery(" << QString().sprintf("%8p", this) << ")";

	// Set ivar defaults
	_limit = -1;
	_skip = -1;
	_count = -1;
	_getObjectReply = NULL;
	_findReply = NULL;
	_getFirstObjectReply = NULL;
	_countReply = NULL;
}

PFQuery::~PFQuery()
{
	qDebug().nospace() << "Destroyed PFQuery(" << QString().sprintf("%8p", this) << ")";
}

#pragma mark - Creation Methods

PFQueryPtr PFQuery::queryWithClassName(const QString& className)
{
	if (className.isEmpty())
	{
		qWarning() << "PFQuery::queryWithClassName failed to create a new PFQuery because the className was empty";
		return PFQueryPtr();
	}
	else
	{
		PFQueryPtr query = PFQueryPtr(new PFQuery(), &QObject::deleteLater);
		query->_className = className;
		return query;
	}
}

#pragma mark - Query Options

void PFQuery::includeKey(const QString& key)
{
	_includeKeys.insert(key);
}

void PFQuery::selectKeys(const QStringList& keys)
{
	_selectKeys |= keys.toSet();
}

#pragma mark - Key Constraints

void PFQuery::whereKeyEqualTo(const QString& key, const QVariant& object)
{
	_whereMap[key] = object;
	_whereEqualKeys.insert(key);
}

void PFQuery::whereKeyNotEqualTo(const QString& key, const QVariant& object)
{
	addWhereOption(key, "$ne", object);
}

#pragma mark - Sorting Methods

void PFQuery::orderByAscending(const QString& key)
{
	_orderKeys.clear();
	_orderKeys.append(key);
}

void PFQuery::orderByDescending(const QString& key)
{
	_orderKeys.clear();
	_orderKeys.append(QString("-") + key);
}

void PFQuery::addAscendingOrder(const QString& key)
{
	_orderKeys.append(key);
}

void PFQuery::addDescendingOrder(const QString& key)
{
	_orderKeys.append(QString("-") + key);
}

#pragma mark - Pagination Methods

void PFQuery::setLimit(int limit)
{
	_limit = limit;
}

int PFQuery::limit()
{
	return _limit;
}

void PFQuery::setSkip(int skip)
{
	_skip = skip;
}

int PFQuery::skip()
{
	return _skip;
}

#pragma mark - Get Object Methods

PFObjectPtr PFQuery::getObjectOfClassWithId(const QString& className, const QString& objectId)
{
	PFErrorPtr error;
	return getObjectOfClassWithId(className, objectId, error);
}

PFObjectPtr PFQuery::getObjectOfClassWithId(const QString& className, const QString& objectId, PFErrorPtr& error)
{
	PFQueryPtr query = PFQuery::queryWithClassName(className);
	if (query.isNull())
		return PFObjectPtr();
	else
		return query->getObjectWithId(objectId, error);
}

PFObjectPtr PFQuery::getObjectWithId(const QString& objectId)
{
	PFErrorPtr error;
	return getObjectWithId(objectId, error);
}

PFObjectPtr PFQuery::getObjectWithId(const QString& objectId, PFErrorPtr& error)
{
	// Reset the where map and add the object id key
	_whereMap.clear();
	_whereEqualKeys.clear();
	whereKeyEqualTo("objectId", objectId);

	// Prep the request and data
	QNetworkRequest networkRequest = createGetObjectNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	PFObjectPtr object = deserializeGetObjectNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();

	return object;
}

void PFQuery::getObjectWithIdInBackground(const QString& objectId, QObject* getObjectCompleteTarget, const char* getObjectCompleteAction)
{
	// Reset the where map and add the object id key
	_whereMap.clear();
	_whereEqualKeys.clear();
	whereKeyEqualTo("objectId", objectId);

	// Prep the request and data
	QNetworkRequest networkRequest = createGetObjectNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_getObjectReply = networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(_getObjectReply, SIGNAL(finished()), this, SLOT(handleGetObjectCompleted()));
	QObject::connect(this, SIGNAL(getObjectCompleted(PFObjectPtr, PFErrorPtr)), getObjectCompleteTarget, getObjectCompleteAction);
}

#pragma mark - Get User Methods
PFUserPtr PFQuery::getUserWithId(const QString& objectId)
{
	PFErrorPtr error;
	return getUserWithId(objectId, error);
}

PFUserPtr PFQuery::getUserWithId(const QString& objectId, PFErrorPtr& error)
{
	// Create a temp query and set the object id
	PFQueryPtr query = PFQuery::queryWithClassName("_User");
	query->whereKeyEqualTo("objectId", objectId);

	// Prep the request and data
	QNetworkRequest networkRequest = query->createGetUserNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	PFUserPtr user = query->deserializeGetUserNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();

	return user;
}

#pragma mark - Find Objects Methods

PFObjectList PFQuery::findObjects()
{
	PFErrorPtr error;
	return findObjects(error);
}

PFObjectList PFQuery::findObjects(PFErrorPtr& error)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createFindObjectsNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	const PFObjectList& objects = deserializeFindObjectsNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();

	return objects;
}

void PFQuery::findObjectsInBackground(QObject* findCompleteTarget, const char* findCompleteAction)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createFindObjectsNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_findReply = networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(_findReply, SIGNAL(finished()), this, SLOT(handleFindObjectsCompleted()));
	QObject::connect(this, SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)), findCompleteTarget, findCompleteAction);
}

#pragma mark - Get First Object Methods

PFObjectPtr PFQuery::getFirstObject()
{
	PFErrorPtr error;
	return getFirstObject(error);
}

PFObjectPtr PFQuery::getFirstObject(PFErrorPtr& error)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createGetFirstObjectNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	PFObjectPtr object = deserializeGetFirstObjectNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();

	return object;
}

void PFQuery::getFirstObjectInBackground(QObject* getCompleteTarget, const char* getCompleteAction)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createFindObjectsNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_getFirstObjectReply = networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(_getFirstObjectReply, SIGNAL(finished()), this, SLOT(handleGetFirstObjectCompleted()));
	QObject::connect(this, SIGNAL(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)), getCompleteTarget, getCompleteAction);
}

#pragma mark - Count Objects Methods

int PFQuery::countObjects()
{
	PFErrorPtr error;
	return countObjects(error);
}

int PFQuery::countObjects(PFErrorPtr& error)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createCountObjectsNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	int count = deserializeCountObjectsNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();

	return count;
}

void PFQuery::countObjectsInBackground(QObject* countCompleteTarget, const char* countCompleteAction)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createCountObjectsNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_countReply = networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(_countReply, SIGNAL(finished()), this, SLOT(handleCountObjectsCompleted()));
	QObject::connect(this, SIGNAL(countObjectsCompleted(int, PFErrorPtr)), countCompleteTarget, countCompleteAction);
}

#pragma mark - Cancel Methods

void PFQuery::cancel()
{
	if (_getObjectReply)
	{
		qDebug() << "Cancelling PFQuery get object operation";
		disconnect(SIGNAL(getObjectCompleted(PFObjectPtr, PFErrorPtr)));
		_getObjectReply->disconnect();
		_getObjectReply->abort();
		_getObjectReply->deleteLater();
	}

	if (_findReply)
	{
		qDebug() << "Cancelling PFQuery find objects operation";
		disconnect(SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)));
		_findReply->disconnect();
		_findReply->abort();
		_findReply->deleteLater();
	}

	if (_getFirstObjectReply)
	{
		qDebug() << "Cancelling PFQuery get first object operation";
		disconnect(SIGNAL(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)));
		_getFirstObjectReply->disconnect();
		_getFirstObjectReply->abort();
		_getFirstObjectReply->deleteLater();
	}

	if (_countReply)
	{
		qDebug() << "Cancelling PFQuery count objects operation";
		disconnect(SIGNAL(countObjectsCompleted(int, PFErrorPtr)));
		_countReply->disconnect();
		_countReply->abort();
		_countReply->deleteLater();
	}
}

#pragma mark - Accessor Methods

const QString& PFQuery::className()
{
	return _className;
}

#pragma mark - Background Network Reply Completion Slots

void PFQuery::handleGetObjectCompleted()
{
	// Disconnect the get object reply from this instance
	_getObjectReply->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	PFObjectPtr object = deserializeGetObjectNetworkReply(_getObjectReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit getObjectCompleted(object, error);
	this->disconnect(SIGNAL(getObjectCompleted(PFObjectPtr, PFErrorPtr)));

	// Clean up
	_getObjectReply->deleteLater();
}

void PFQuery::handleFindObjectsCompleted()
{
	// Disconnect the find reply from this instance
	_findReply->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	const PFObjectList& objects = deserializeFindObjectsNetworkReply(_findReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit findObjectsCompleted(objects, error);
	this->disconnect(SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)));

	// Clean up
	_findReply->deleteLater();
}

void PFQuery::handleGetFirstObjectCompleted()
{
	// Disconnect the get first object reply from this instance
	_getFirstObjectReply->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	PFObjectPtr object = deserializeGetFirstObjectNetworkReply(_getFirstObjectReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit getFirstObjectCompleted(object, error);
	this->disconnect(SIGNAL(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)));

	// Clean up
	_getFirstObjectReply->deleteLater();
}

void PFQuery::handleCountObjectsCompleted()
{
	// Disconnect the count reply from this instance
	_countReply->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	int count = deserializeCountObjectsNetworkReply(_countReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit countObjectsCompleted(count, error);
	this->disconnect(SIGNAL(countObjectsCompleted(int, PFErrorPtr)));

	// Clean up
	_countReply->deleteLater();
}

#pragma mark - Network Request Builder Methods

QNetworkRequest PFQuery::createGetObjectNetworkRequest()
{
	return buildDefaultNetworkRequest();
}

QNetworkRequest PFQuery::createGetUserNetworkRequest()
{
	return buildDefaultNetworkRequest();
}

QNetworkRequest PFQuery::createFindObjectsNetworkRequest()
{
	return buildDefaultNetworkRequest();
}

QNetworkRequest PFQuery::createGetFirstObjectNetworkRequest()
{
	// Force the limit to 1
	_limit = 1;

	return buildDefaultNetworkRequest();
}

QNetworkRequest PFQuery::createCountObjectsNetworkRequest()
{
	// Set the limit and count
	_limit = 0;
	_count = 1;

	return buildDefaultNetworkRequest();
}

#pragma mark - Network Reply Deserialization Methods

PFObjectPtr PFQuery::deserializeGetObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Just use the find objects deserialization since they are the same
	PFObjectList objects = deserializeFindObjectsNetworkReply(networkReply, error);
	PFObjectPtr object;
	if (!objects.isEmpty())
		object = objects.at(0);

	return object;
}

PFUserPtr PFQuery::deserializeGetUserNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Create the object to return
	PFUserPtr user;

	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the results array
		QJsonObject rootObject = doc.object();
		QJsonArray results = rootObject["results"].toArray();

		// Convert the first result to a PFUser if it exists
		if (!results.isEmpty())
		{
			// Grab the result json user
			QJsonValue resultValue = results.at(0);
			QJsonObject resultUser = resultValue.toObject();

			// Convert the json to a PFUser
			QVariant userVariant = PFUser::fromJson(resultUser);
			user = PFUser::userFromVariant(userVariant);
		}
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
	}

	return user;
}

PFObjectList PFQuery::deserializeFindObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Create the list of objects to return
	PFObjectList objects;

	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the results array
		QJsonObject rootObject = doc.object();
		QJsonArray results = rootObject["results"].toArray();

		// Go through each item in the reply and create a PFObject out of it
		foreach (const QJsonValue& resultValue, results)
		{
			// Grab the result json object and add the className property to it
			QJsonObject resultObject = resultValue.toObject();
			resultObject["className"] = _className;

			// Convert the json to a PFObject and store it
			QVariant objectVariant = PFObject::fromJson(resultObject);
			PFObjectPtr object = PFObject::objectFromVariant(objectVariant);
			objects.append(object);
		}
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
	}

	return objects;
}

PFObjectPtr PFQuery::deserializeGetFirstObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Use the get object deserialization since it's exactly the same
	return deserializeGetObjectNetworkReply(networkReply, error);
}

int PFQuery::deserializeCountObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	int count = -1;
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the count int
		QJsonObject rootObject = doc.object();
		count = rootObject["count"].toInt();
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
	}

	return count;
}

#pragma mark - Protected Helper Methods

void PFQuery::addWhereOption(const QString& key, const QString& option, const QVariant& object)
{
	// Remove the equal key and object if it was previously set to be an equal key
	if (_whereEqualKeys.contains(key))
	{
		_whereMap.remove(key);
		_whereEqualKeys.remove(key);
	}

	// Fetch the previous key map or create a new one
	QVariantMap keyMap = _whereMap.value(key, QVariantMap()).toMap();

	// Add the new options to the key map and update the where map
	keyMap[option] = object;
	_whereMap[key] = keyMap;
}

QNetworkRequest PFQuery::buildDefaultNetworkRequest()
{
	// Create the url
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className);
	if (_className == PFUSER_QUERY_CLASSNAME)
		url = QUrl(QString("https://api.parse.com/1/users"));

	// Create the url query
	QUrlQuery urlQuery;

	// Attach the "where" query
	if (!_whereMap.isEmpty())
	{
		QJsonObject whereJsonObject = PFConversion::convertVariantToJson(_whereMap).toObject();
		QString whereJsonString = QString::fromUtf8(QJsonDocument(whereJsonObject).toJson(QJsonDocument::Compact));
		urlQuery.addQueryItem("where", whereJsonString);
	}

	// Attach the "include" query
	if (!_includeKeys.isEmpty())
	{
		QStringList includeKeys = _includeKeys.toList();
		QString includeString = includeKeys.join(",");
		urlQuery.addQueryItem("include", includeString);
	}

	// Attach the "keys" query (select keys)
	if (!_selectKeys.isEmpty())
	{
		QStringList selectKeys = _selectKeys.toList();
		QString selectString = selectKeys.join(",");
		urlQuery.addQueryItem("keys", selectString);
	}

	// Attach the "order" query
	if (!_orderKeys.isEmpty())
	{
		QString orderString = _orderKeys.join(",");
		urlQuery.addQueryItem("order", orderString);
	}

	// Attach the "limit" query
	if (_limit != -1)
	{
		QString limitString = QString::number(_limit);
		urlQuery.addQueryItem("limit", limitString);
	}

	// Attach the "skip" query
	if (_skip != -1)
	{
		QString skipString = QString::number(_skip);
		urlQuery.addQueryItem("skip", skipString);
	}

	// Attach the "count" query
	if (_count != -1)
	{
		QString countString = QString::number(_count);
		urlQuery.addQueryItem("count", countString);
	}

	// Attach the url query to the url
	url.setQuery(urlQuery);

	// Create the request
	QNetworkRequest networkRequest(url);

	// Attach the necessary raw headers
	networkRequest.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	networkRequest.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		networkRequest.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return networkRequest;
}

}	// End of parse namespace
