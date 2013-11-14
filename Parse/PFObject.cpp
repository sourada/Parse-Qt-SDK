//
//  PFObject.cpp
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include "PFACL.h"
#include "PFDateTime.h"
#include "PFError.h"
#include "PFFile.h"
#include "PFManager.h"
#include "PFObject.h"
#include "PFUser.h"

// Qt headers
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

namespace parse {

#pragma mark - Memory Management Methods

PFObject::PFObject()
{
	qDebug().nospace() << "Created PFObject(" << QString().sprintf("%8p", this) << ")";

	_parseClassName = "";
	_objectId = "";
	_acl = PFACLPtr();
	_createdAt = PFDateTimePtr();
	_updatedAt = PFDateTimePtr();
	_childObjects = QVariantMap();
	_updatedChildObjects = QVariantMap();
	_isSaving = false;
	_isDeleting = false;
}

PFObject::~PFObject()
{
	qDebug().nospace() << "Destroyed PFObject(" << QString().sprintf("%8p", this) << ")";
}

#pragma mark - Creation Methods

PFObjectPtr PFObject::objectWithClassName(const QString& className)
{
	if (className.isEmpty())
	{
		qWarning() << "PFObject::objectWithClassName failed to create new PFObject because the className is empty";
		return PFObjectPtr();
	}
	else
	{
		// Create a new PFObject and set the parse class name
		PFObjectPtr object = PFObjectPtr(new PFObject(), &QObject::deleteLater);
		object->_parseClassName = className;

		// Add the default acl if set
		PFACLPtr defaultACL;
		bool currentUserAccess;
		PFACL::defaultACLWithCurrentUserAccess(defaultACL, currentUserAccess);
		if (!defaultACL.isNull())
		{
			// Modify the default acl if the current user access is set
			if (currentUserAccess)
			{
				PFUserPtr currentUser = PFUser::currentUser();
				if (!currentUser.isNull())
				{
					defaultACL = defaultACL->clone();
					defaultACL->setReadAccessForUser(true, currentUser);
					defaultACL->setWriteAccessForUser(true, currentUser);
				}
			}

			object->setACL(defaultACL);
		}

		return object;
	}
}

PFObjectPtr PFObject::objectWithClassName(const QString& className, const QString& objectId)
{
	if (className.isEmpty() || objectId.isEmpty())
	{
		qWarning() << "PFObject::objectWithClassName failed to create new PFObject because the className and/or the objectId is empty";
		return PFObjectPtr();
	}
	else
	{
		PFObjectPtr object = PFObjectPtr(new PFObject(), &QObject::deleteLater);
		object->_parseClassName = className;
		object->_objectId = objectId;

		return object;
	}
}

PFObjectPtr PFObject::objectFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = PFSerializable::fromVariant(variant);
	if (!serializable.isNull())
		return serializable.objectCast<PFObject>();

	return PFObjectPtr();
}

#pragma mark - Object Storage Methods

void PFObject::setObjectForKey(const QVariant& object, const QString& key)
{
	_childObjects[key] = object;
	if (!_objectId.isEmpty())
		_updatedChildObjects[key] = object;
}

void PFObject::setObjectForKey(PFSerializablePtr object, const QString& key)
{
	setObjectForKey(PFSerializable::toVariant(object), key);
}

const QVariant& PFObject::objectForKey(const QString& key)
{
	return _childObjects[key];
}

QStringList PFObject::allKeys()
{
	return _childObjects.keys();
}

#pragma mark - ACL Accessor Methods

void PFObject::setACL(PFACLPtr acl)
{
	_acl = acl;
	setObjectForKey(PFSerializable::toVariant(acl), "ACL");
}

PFACLPtr PFObject::ACL()
{
	return _acl;
}

#pragma mark - Object Info Getter Methods

const QString PFObject::parseClassName()
{
	return _parseClassName;
}

const QString& PFObject::objectId()
{
	return _objectId;
}

PFDateTimePtr PFObject::createdAt()
{
	return _createdAt;
}

PFDateTimePtr PFObject::updatedAt()
{
	return _updatedAt;
}

#pragma mark - Save Methods

bool PFObject::save()
{
	PFErrorPtr error;
	return save(error);
}

bool PFObject::save(PFErrorPtr& error)
{
	// Early out if the object is already saving
	if (_isSaving)
	{
		qWarning().nospace() << "WARNING: PFObject is already being saved";
		return false;
	}

	// Update the ivar
	_isSaving = true;

	// Prep the request and data
	bool updateRequired = needsUpdate();
	QNetworkRequest request;
	QByteArray data;
	createSaveNetworkRequest(request, data);

	// Execute the request and connect the callbacks
	QNetworkReply* networkReply = NULL;
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	if (updateRequired)
		networkReply = networkAccessManager->put(request, data);
	else
		networkReply = networkAccessManager->post(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = deserializeSaveNetworkReply(networkReply, updateRequired, error);

	// Clear out the updated children if necessary
	if (updateRequired && success)
		_updatedChildObjects.clear();

	// Update the ivar
	_isSaving = false;

	// Clean up
	networkReply->deleteLater();

	return success;
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
	bool updateRequired = needsUpdate();
	QNetworkRequest request;
	QByteArray data;
	createSaveNetworkRequest(request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	if (updateRequired)
		networkAccessManager->put(request, data);
	else
		networkAccessManager->post(request, data);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleSaveCompleted(QNetworkReply*)));
	QObject::connect(this, SIGNAL(saveCompleted(bool, PFErrorPtr)), saveCompleteTarget, saveCompleteAction);

	return true;
}

#pragma mark - Delete Object Methods

bool PFObject::deleteObject()
{
	PFErrorPtr error;
	return deleteObject(error);
}

bool PFObject::deleteObject(PFErrorPtr& error)
{
	// Early out if the object is already being deleted
	if (_isDeleting)
	{
		qWarning().nospace() << "WARNING: PFObject is already being deleted";
		return false;
	}

	// Update the ivar
	_isDeleting = true;

	// Create the network request
	QNetworkRequest networkRequest = createDeleteObjectNetworkRequest();

	// Execute the network request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->deleteResource(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = deserializeDeleteObjectReply(networkReply, error);

	// Update the ivars
	_isDeleting = false;
	_objectId = "";

	// Clean up
	networkReply->deleteLater();

	return success;
}

bool PFObject::deleteObjectInBackground(QObject *deleteObjectCompleteTarget, const char *deleteObjectCompleteAction)
{
	// Early out if the object is already being deleted
	if (_isDeleting)
	{
		qWarning().nospace() << "WARNING: PFObject is already being deleted";
		return false;
	}

	// Update the ivar
	_isDeleting = true;

	// Create the network request
	QNetworkRequest networkRequest = createDeleteObjectNetworkRequest();

	// Execute the network request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->deleteResource(networkRequest);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleDeleteObjectCompleted(QNetworkReply*)));
	QObject::connect(this, SIGNAL(deleteObjectCompleted(bool, PFErrorPtr)), deleteObjectCompleteTarget, deleteObjectCompleteAction);

	return true;
}

#pragma mark - PFSerializable Methods

PFSerializablePtr PFObject::fromJson(const QJsonObject& jsonObject)
{
	Q_UNUSED(jsonObject);
	qDebug() << "PFFile::fromJSON NOT implemented!!!";
	return PFSerializablePtr();
}

bool PFObject::toJson(QJsonObject& jsonObject)
{
	qDebug() << "PFObject::toJson";
	if (_objectId.isEmpty())
	{
		qWarning() << "PFObject::toJson could NOT convert to PFObject to JSON because the _objectId is not set";
		return false;
	}
	else
	{
		jsonObject["__type"] = QString("Pointer");
		jsonObject["className"] = _parseClassName;
		jsonObject["objectId"] = _objectId;
		return true;
	}
}

const QString PFObject::className() const
{
	return "PFObject";
}

#pragma mark - Background Request Completion Signals

void PFObject::handleSaveCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	bool updated = needsUpdate();
	PFErrorPtr error;
	bool success = deserializeSaveNetworkReply(networkReply, updated, error);

	// Clear out the updated children if necessary
	if (updated && success)
		_updatedChildObjects.clear();

	// Update the ivar
	_isSaving = false;

	// Emit the signal that the save has completed and then disconnect it
	emit saveCompleted(success, error);
	this->disconnect(SIGNAL(saveCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

void PFObject::handleDeleteObjectCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeDeleteObjectReply(networkReply, error);

	// Update the ivars
	_isDeleting = false;
	_objectId = "";

	// Emit the signal that the delete object has completed and then disconnect it
	emit deleteObjectCompleted(success, error);
	this->disconnect(SIGNAL(deleteObjectCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

#pragma mark - Protected Methods

bool PFObject::needsUpdate()
{
	return !_objectId.isEmpty();
}

#pragma mark - Network Request Builder Methods

void PFObject::createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data)
{
	// Create the url based on whether we should create or update the PFObject
	bool updateRequired = needsUpdate();
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _parseClassName);
	if (updateRequired)
		url = QUrl(url.toString() + "/" + _objectId);

	// Create a network request
	request = QNetworkRequest(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Figure out whether we need to
	QVariantMap objectsToSerialize;
	if (updateRequired)
		objectsToSerialize = _updatedChildObjects;
	else
		objectsToSerialize = _childObjects;

	// Serialize all the objects into json
	QJsonObject jsonObject;
	foreach (const QString& key, objectsToSerialize.keys())
	{
		QVariant objectToSerialize = objectsToSerialize[key];
		jsonObject[key] = convertDataToJson(objectToSerialize);
	}
	data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
	qDebug() << "Data: " << data;
}

QJsonValue PFObject::convertDataToJson(const QVariant& data)
{
	if ((QMetaType::Type) data.type() == QMetaType::QVariantList)
	{
		QJsonArray jsonArray;
		foreach (const QVariant& dataObject, data.toList())
		{
			QJsonValue jsonValue = convertDataToJson(dataObject);
			jsonArray.append(jsonValue);
		}

		return QJsonValue(jsonArray);
	}
	else if ((QMetaType::Type) data.type() == QMetaType::QVariantMap)
	{
		QJsonObject jsonObject;
		QVariantMap dataMap;
		foreach (const QString& key, dataMap.keys())
		{
			QVariant dataObject = dataMap[key];
			jsonObject[key] = convertDataToJson(dataObject);
		}

		return QJsonValue(jsonObject);
	}
	else if ((QMetaType::Type) data.type() == QMetaType::QVariantHash)
	{
		QJsonObject jsonObject;
		QVariantHash dataHash;
		foreach (const QString& key, dataHash.keys())
		{
			QVariant dataObject = dataHash[key];
			jsonObject[key] = convertDataToJson(dataObject);
		}

		return QJsonValue(jsonObject);
	}
	else if (data.canConvert<PFSerializablePtr>())	// PFSerializablePtr
	{
		PFSerializablePtr serializable = data.value<PFSerializablePtr>();
		QJsonObject jsonObject;
		serializable->toJson(jsonObject);
		return QJsonValue(jsonObject);
	}
	else
	{
		return QJsonValue::fromVariant(data);
	}
}

QNetworkRequest PFObject::createDeleteObjectNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _parseClassName + "/" + _objectId);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	return request;
}

#pragma mark - Network Reply Deserialization Methods

bool PFObject::deserializeSaveNetworkReply(QNetworkReply* networkReply, bool updated, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		if (!updated) // SAVED
		{
			QString createdAt = jsonObject["createdAt"].toString();
			_createdAt = PFDateTime::dateTimeFromParseString(createdAt);
			_objectId = jsonObject["objectId"].toString();
			qDebug().nospace() << "Created Object:" << _parseClassName << " with objectId:" << _objectId;
		}
		else // UPDATED
		{
			QString updatedAt = jsonObject["updatedAt"].toString();
			_updatedAt = PFDateTime::dateTimeFromParseString(updatedAt);
			qDebug().nospace() << "Updated Object:" << _parseClassName << " with objectId:" << _objectId;
		}

		return true;
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

bool PFObject::deserializeDeleteObjectReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Payload is empty on success
		return true;
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

}	// End of parse namespace
