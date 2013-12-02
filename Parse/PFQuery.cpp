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

#pragma mark - Memory Management Methods

PFQuery::PFQuery()
{
	qDebug().nospace() << "Created PFQuery(" << QString().sprintf("%8p", this) << ")";

	// Set ivar defaults
	_limit = -1;
	_skip = -1;
	_findReply = NULL;
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
	networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleGetObjectCompleted(QNetworkReply*)));
	QObject::connect(this, SIGNAL(getObjectCompleted(PFObjectPtr, PFErrorPtr)), getObjectCompleteTarget, getObjectCompleteAction);
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

#pragma mark - Cancel Methods

void PFQuery::cancel()
{
	if (_findReply)
	{
		qDebug() << "Cancelling PFQuery find objects operation";
		disconnect(SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)));
		_findReply->disconnect();
		_findReply->abort();
		_findReply->deleteLater();
	}
}

#pragma mark - Accessor Methods

const QString& PFQuery::className()
{
	return _className;
}

#pragma mark - Background Network Reply Completion Slots

void PFQuery::handleGetObjectCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	PFObjectPtr object = deserializeGetObjectNetworkReply(networkReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit getObjectCompleted(object, error);
	this->disconnect(SIGNAL(getObjectCompleted(PFObjectPtr, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
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

#pragma mark - Network Request Builder Methods

QNetworkRequest PFQuery::createGetObjectNetworkRequest()
{
	// Create the url
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className);
	QUrlQuery urlQuery;

	// Attach the "where" query
	QJsonObject whereJsonObject = PFConversion::convertVariantToJson(_whereMap).toObject();
	QString whereJsonString = QString::fromUtf8(QJsonDocument(whereJsonObject).toJson(QJsonDocument::Compact));
	urlQuery.addQueryItem("where", whereJsonString);

	// Attach the url query to the url
	url.setQuery(urlQuery);

	// Create the request
	QNetworkRequest request(url);

	// Attach the necessary raw headers
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

QNetworkRequest PFQuery::createFindObjectsNetworkRequest()
{
	// Create the url
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className);
	QUrlQuery urlQuery;

	// Attach the "where" query
	if (!_whereMap.isEmpty())
	{
		QJsonObject whereJsonObject = PFConversion::convertVariantToJson(_whereMap).toObject();
		QString whereJsonString = QString::fromUtf8(QJsonDocument(whereJsonObject).toJson(QJsonDocument::Compact));
		urlQuery.addQueryItem("where", whereJsonString);
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

	// Attach the url query to the url
	url.setQuery(urlQuery);

	// Create the request
	QNetworkRequest request(url);

	// Attach the necessary raw headers
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
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

#pragma mark - Key Helper Methods

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

}	// End of parse namespace
