//
//  PFObject.cpp
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include <Parse/PFManager.h>
#include <Parse/PFObject.h>

// Qt headers
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace parse {

#pragma mark - Memory Management Methods

PFObject::PFObject(const QString& className)
{
	initialize();
	_parseClassName = className;
}

PFObject::PFObject(const QString& className, const QString& objectId)
{
	initialize();
	_parseClassName = className;
	_objectId = objectId;
}

void PFObject::initialize()
{
	_parseClassName = "";
	_objectId = "";
	_createdAt = PFDateTime();
	_updatedAt = PFDateTime();
	_primitiveObjects = QVariantMap();
	_updatedPrimitiveObjects = QVariantMap();
	_isSaving = false;
	_saveReply = NULL;
}

PFObject::~PFObject()
{
	// No-op
	qDebug() << "PFObject destroyed";
}

PFObjectPtr PFObject::objectWithClassName(const QString& className)
{
	return PFObjectPtr(new PFObject(className));
}

PFObjectPtr PFObject::objectWithClassName(const QString& className, const QString& objectId)
{
	return PFObjectPtr(new PFObject(className, objectId));
}

void PFObject::setObjectForKey(const QVariant& object, const QString& key)
{
	_primitiveObjects[key] = object;
	if (!_objectId.isEmpty())
		_updatedPrimitiveObjects[key] = object;
}

const QVariant& PFObject::objectForKey(const QString& key)
{
	return _primitiveObjects[key];
}

QList<QString> PFObject::allKeys()
{
	return _primitiveObjects.keys();
}

bool PFObject::save()
{
	PFErrorPtr error;
	return save(error);
}

bool PFObject::save(PFErrorPtr& error)
{
	// Early out if the file is already saving
	if (_isSaving)
	{
		qWarning().nospace() << "WARNING: PFObject is already being saved";
		return false;
	}

	// Update the ivar
	_isSaving = true;

	// Prep the request and data
	bool updateRequired = needsUpdated();
	QNetworkRequest request = buildSaveNetworkRequest();
	QByteArray data = buildSaveData();

	// Execute the request and connect the callbacks
	QNetworkReply* reply = NULL;
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	if (!updateRequired)
		reply = networkAccessManager->post(request, data);
	else
		reply = networkAccessManager->put(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Handle the reply
	error = parseSaveNetworkReply(reply, updateRequired);
	bool succeeded = error.isNull();

	// Clear out the updated primitives if necessary
	if (updateRequired && succeeded)
		_updatedPrimitiveObjects.clear();

	// Update the ivar
	_isSaving = false;

	return succeeded;
}

bool PFObject::saveInBackground(QObject *saveCompleteTarget, const char *saveCompleteAction)
{
	// Early out if the file is already saving
	if (_isSaving)
	{
		qWarning().nospace() << "WARNING: PFObject is already being saved";
		return false;
	}

	// Update the ivar
	_isSaving = true;

	// Prep the request and data
	bool updateRequired = needsUpdated();
	QNetworkRequest request = buildSaveNetworkRequest();
	QByteArray data = buildSaveData();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	if (!updateRequired)
		_saveReply = networkAccessManager->post(request, data);
	else
		_saveReply = networkAccessManager->put(request, data);
	QObject::connect(_saveReply, SIGNAL(finished()), this, SLOT(handleSaveCompleted()));
	QObject::connect(this, SIGNAL(saveCompleted(PFObject*, bool, PFErrorPtr)), saveCompleteTarget, saveCompleteAction);

	return true;
}

void PFObject::handleSaveCompleted()
{
	// Handle the reply
	bool updateRequired = needsUpdated();
	PFErrorPtr error = parseSaveNetworkReply(_saveReply, updateRequired);
	bool succeeded = error.isNull();

	// Clear out the updated primitives if necessary
	if (updateRequired && succeeded)
		_updatedPrimitiveObjects.clear();

	// Update the ivar
	_isSaving = false;

	// Notify the target
	emit saveCompleted(this, succeeded, error);

	// Clean up
	_saveReply->deleteLater();
}

bool PFObject::needsUpdated()
{
	return !_objectId.isEmpty();
}

QNetworkRequest PFObject::buildSaveNetworkRequest()
{
	// Create the url based on whether we should create or update the PFObject
	bool updateRequired = needsUpdated();
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _parseClassName);
	if (updateRequired)
		url = QUrl(url.toString() + "/" + _objectId);

	// Create a network request
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	return request;
}

QByteArray PFObject::buildSaveData()
{
	// Convert the primitives to json
	QJsonObject jsonObject;
	if (!needsUpdated())
		jsonObject = QJsonObject::fromVariantMap(_primitiveObjects);
	else
		jsonObject = QJsonObject::fromVariantMap(_updatedPrimitiveObjects);
	QByteArray data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
	qDebug() << "Data: " << data;

	return data;
}

PFErrorPtr PFObject::parseSaveNetworkReply(QNetworkReply* networkReply, bool updated)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Notify the target of the success or failure
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		if (!updated) // SAVED
		{
			QString createdAt = jsonObject["createdAt"].toString();
			_createdAt = PFDateTime::fromParseString(createdAt);
			_objectId = jsonObject["objectId"].toString();
			qDebug().nospace() << "Created Object:" << _parseClassName << " with objectId:" << _objectId;
		}
		else // UPDATED
		{
			QString updatedAt = jsonObject["updatedAt"].toString();
			_updatedAt = PFDateTime::fromParseString(updatedAt);
			qDebug().nospace() << "Updated Object:" << _parseClassName << " with objectId:" << _objectId;
		}

		return PFErrorPtr();
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		return PFErrorPtr(new PFError(errorCode, errorMessage));
	}
}

}	// End of parse namespace
