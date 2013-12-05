//
//  PFObject.cpp
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFACL.h"
#include "PFConversion.h"
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

// Static Globals
static QHash<PFObject *, PFObjectList> gActiveBackgroundObjects; // Used for save all, delete all and fetch all

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFObject::PFObject()
{
	qDebug().nospace() << "Created PFObject(" << QString().sprintf("%8p", this) << ")";

	_className = "";
	_objectId = "";
	_acl = PFACLPtr();
	_createdAt = PFDateTimePtr();
	_updatedAt = PFDateTimePtr();
	_properties = QVariantMap();
	_updatedProperties = QVariantMap();
	_isSaving = false;
	_isDeleting = false;
	_isFetching = false;
	_fetched = false;
}

PFObject::~PFObject()
{
	qDebug().nospace() << "Destroyed PFObject(" << QString().sprintf("%8p", this) << ")";
}

#ifdef __APPLE__
#pragma mark - Creation Methods
#endif

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
		object->_className = className;

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
		object->_className = className;
		object->_objectId = objectId;

		return object;
	}
}

PFObjectPtr PFObject::objectFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = variant.value<PFSerializablePtr>();
	if (!serializable.isNull())
		return serializable.objectCast<PFObject>();

	return PFObjectPtr();
}

#ifdef __APPLE__
#pragma mark - Object Storage Methods
#endif

void PFObject::setObjectForKey(const QVariant& object, const QString& key)
{
	_properties[key] = object;
	if (!_objectId.isEmpty())
		_updatedProperties[key] = object;
}

void PFObject::setObjectForKey(PFSerializablePtr object, const QString& key)
{
	setObjectForKey(toVariant(object), key);
}

bool PFObject::removeObjectForKey(const QString& key)
{
	if (_properties.contains(key))
	{
		// Remove the property
		_properties.remove(key);

		// Create a delete operation if the object is not new
		if (!_objectId.isEmpty())
		{
			QVariantMap operation;
			operation["__op"] = QString("Delete");
			_updatedProperties[key] = operation;
		}

		return true;
	}

	return false;
}

QVariant PFObject::objectForKey(const QString& key)
{
	if (_properties.contains(key))
		return _properties[key];

	return QVariant();
}

QStringList PFObject::allKeys()
{
	return _properties.keys();
}

#ifdef __APPLE__
#pragma mark - Increment Methods
#endif

void PFObject::incrementKey(const QString& key)
{
	incrementKeyByAmount(key, 1);
}

void PFObject::incrementKeyByAmount(const QString& key, int amount)
{
	if (_properties.contains(key))
	{
		// Create a set of supported meta types for the variant
		QSet<QMetaType::Type> supportedTypes;
		supportedTypes << QMetaType::Char;
		supportedTypes << QMetaType::UChar;
		supportedTypes << QMetaType::Short;
		supportedTypes << QMetaType::UShort;
		supportedTypes << QMetaType::Int;
		supportedTypes << QMetaType::UInt;
		supportedTypes << QMetaType::Long;
		supportedTypes << QMetaType::ULong;
		supportedTypes << QMetaType::LongLong;
		supportedTypes << QMetaType::ULongLong;
		supportedTypes << QMetaType::Float;
		supportedTypes << QMetaType::Double;

		// Only increment the property if it is actually a number
		QVariant property = _properties[key];
		QMetaType::Type propertyType = (QMetaType::Type) property.type();
		if (supportedTypes.contains(propertyType))
		{
			// Update the property value and push it back into the properties map
			double value = property.toDouble();
			value += amount;
			QVariant newVariant = value;
			newVariant.convert(propertyType);
			_properties[key] = newVariant;

			// Create an increment operation if the object is not new
			if (!_objectId.isEmpty())
			{
				QVariantMap operation;
				operation["__op"] = QString("Increment");
				operation["amount"] = amount;
				_updatedProperties[key] = operation;
			}
		}
		else
		{
			qWarning().nospace() << "PFObject::incrementKeyByAmount failed because the property for key: " << key << " is NOT a number";
		}
	}
}

#ifdef __APPLE__
#pragma mark - List Add & Remove Methods
#endif

void PFObject::addObjectToListForKey(const QVariant& object, const QString& key)
{
	QVariantList objects;
	objects.append(object);
	addObjectsToListForKey(objects, key);
}

void PFObject::addObjectToListForKey(PFSerializablePtr object, const QString& key)
{
	addObjectToListForKey(PFSerializable::toVariant(object), key);
}

void PFObject::addObjectsToListForKey(const QVariantList& objects, const QString& key)
{
	if (_properties.contains(key) && !objects.isEmpty())
	{
		QMetaType::Type propertyType = (QMetaType::Type) _properties.value(key).type();
		if (propertyType == QMetaType::QVariantList)
		{
			// Add the objects to the property list
			QVariantList propertyList = _properties.value(key).toList();
			propertyList.append(objects);
			_properties[key] = propertyList;

			// Create an Add operation to append the objects
			if (!_objectId.isEmpty())
			{
				QVariantMap operation;
				operation["__op"] = QString("Add");
				operation["objects"] = objects;
				_updatedProperties[key] = operation;
			}
		}
		else
		{
			qWarning().nospace() << "PFObject::addObjectsToListForKey failed because the object for the given key: " << key << " is NOT a list";
		}
	}
}

void PFObject::addUniqueObjectToListForKey(const QVariant& object, const QString& key)
{
	QVariantList uniqueObjects;
	uniqueObjects.append(object);
	addUniqueObjectsToListForKey(uniqueObjects, key);
}

void PFObject::addUniqueObjectToListForKey(PFSerializablePtr object, const QString& key)
{
	addUniqueObjectToListForKey(PFSerializable::toVariant(object), key);
}

void PFObject::addUniqueObjectsToListForKey(const QVariantList& objects, const QString& key)
{
	if (_properties.contains(key) && !objects.isEmpty())
	{
		QMetaType::Type propertyType = (QMetaType::Type) _properties.value(key).type();
		if (propertyType == QMetaType::QVariantList)
		{
			// Create a property set and find which objects are unique
			QVariantList propertyList = _properties.value(key).toList();

			// Go through all the objects to add and find the unique ones
			QVariantList uniqueObjects = objects;
			foreach (const QVariant& objectToAdd, objects)
			{
				foreach (const QVariant& existingObject, propertyList)
				{
					if (PFConversion::areEqual(existingObject, objectToAdd))
					{
						uniqueObjects.removeOne(objectToAdd);
						break;
					}
				}
			}

			// Only continue if there were unique objects found
			if (!uniqueObjects.isEmpty())
			{
				// Add all the unique objects to the existing property list
				propertyList.append(uniqueObjects);
				_properties[key] = propertyList;

				// Create an Add operation to append the object
				if (!_objectId.isEmpty())
				{
					QVariantMap operation;
					operation["__op"] = QString("AddUnique");
					operation["objects"] = uniqueObjects;
					_updatedProperties[key] = operation;
				}
			}
			else
			{
				qWarning() << "PFObject::addUniqueObjectsToListForKey did not find any unique objects to add to the list";
			}
		}
		else
		{
			qWarning().nospace() << "PFObject::addUniqueObjectsToListForKey failed because the object for the given key: " << key << " is NOT a list";
		}
	}
}

void PFObject::removeObjectFromListForKey(const QVariant& object, const QString& key)
{
	QVariantList objects;
	objects.append(object);
	removeObjectsFromListForKey(objects, key);
}

void PFObject::removeObjectFromListForKey(PFSerializablePtr object, const QString& key)
{
	removeObjectFromListForKey(PFSerializable::toVariant(object), key);
}

void PFObject::removeObjectsFromListForKey(const QVariantList& objects, const QString& key)
{
	if (_properties.contains(key) && !objects.isEmpty())
	{
		// Create a property set and find which objects are unique
		QVariantList currentObjects = _properties.value(key).toList();
		QVariantList matchedObjects;
		foreach (const QVariant& objectToRemove, objects)
		{
			foreach (const QVariant& currentObject, currentObjects)
			{
				if (PFConversion::areEqual(currentObject, objectToRemove))
				{
					matchedObjects.append(currentObject);
					break;
				}
			}
		}

		// Create a remove operation if there are matched objects
		if (!matchedObjects.isEmpty())
		{
			// First remove all the matched objects and update the properties key with the modified list
			foreach (const QVariant& matchedObject, matchedObjects)
				currentObjects.removeAll(matchedObject);
			_properties[key] = currentObjects;

			// Create a Remove operation to remove the objects in the cloud
			if (!_objectId.isEmpty())
			{
				QVariantMap operation;
				operation["__op"] = QString("Remove");
				operation["objects"] = matchedObjects;
				_updatedProperties[key] = operation;
			}
		}
		else
		{
			qWarning() << "PFObject::removeObjectsFromListForKey did not find any objects to remove from the list";
		}
	}
	else
	{
		qWarning().nospace() << "PFObject::removeObjectsFromListForKey failed because the object for the given key: " << key << " is NOT a list";
	}
}

#ifdef __APPLE__
#pragma mark - ACL Accessor Methods
#endif

void PFObject::setACL(PFACLPtr acl)
{
	_acl = acl;
	if (acl.isNull())
		removeObjectForKey("ACL");
	else
		setObjectForKey(toVariant(acl), "ACL");
}

PFACLPtr PFObject::ACL()
{
	return _acl;
}

#ifdef __APPLE__
#pragma mark - Object Info Getter Methods
#endif

const QString PFObject::className()
{
	return _className;
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

#ifdef __APPLE__
#pragma mark - Save Methods
#endif

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
		_updatedProperties.clear();

	// Update the ivar
	_isSaving = false;

	// Clean up
	networkReply->deleteLater();

	return success;
}

bool PFObject::saveInBackground(QObject *target, const char *action)
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
	if (target)
		QObject::connect(this, SIGNAL(saveCompleted(bool, PFErrorPtr)), target, action);

	return true;
}

#ifdef __APPLE__
#pragma mark - Save All Methods
#endif

bool PFObject::saveAll(PFObjectList objects)
{
	PFErrorPtr error;
	return saveAll(objects, error);
}

bool PFObject::saveAll(PFObjectList objects, PFErrorPtr& error)
{
	// Make sure we aren't already saving any of the objects
	foreach (PFObjectPtr object, objects)
	{
		if (object->_isSaving)
		{
			qWarning().nospace() << "WARNING: PFObject is already being saved: " << object->objectId();
			return false;
		}
	}

	// Update the save state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isSaving = true;

	// Create a temp object in order to use the create and deserialize methods
	PFObject* callbackObject = new PFObject();

	// Prep the request and data
	QNetworkRequest request;
	QByteArray data;
	callbackObject->createSaveAllNetworkRequest(objects, request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->post(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = callbackObject->deserializeSaveAllNetworkReply(objects, networkReply, error);

	// Update the save state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isSaving = false;

	// Cleanup
	networkReply->deleteLater();
	callbackObject->deleteLater();

	return success;
}

bool PFObject::saveAllInBackground(PFObjectList objects, QObject *target, const char *action)
{
	// Make sure we aren't already saving any of the objects
	foreach (PFObjectPtr object, objects)
	{
		if (object->_isSaving)
		{
			qWarning().nospace() << "WARNING: PFObject is already being saved: " << object->objectId();
			return false;
		}
	}

	// Update the save state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isSaving = true;

	// Create a temp PFObject on the heap to connect the callbacks which will be cleaned up then
	PFObject* callbackObject = new PFObject();
	gActiveBackgroundObjects.insert(callbackObject, objects);

	// Prep the request and data
	QNetworkRequest request;
	QByteArray data;
	callbackObject->createSaveAllNetworkRequest(objects, request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->post(request, data);

	// Hook up the callbacks to the temp object
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), callbackObject, SLOT(handleSaveAllCompleted(QNetworkReply*)));
	if (target)
		QObject::connect(callbackObject, SIGNAL(saveAllCompleted(bool, PFErrorPtr)), target, action);

	return true;
}

#ifdef __APPLE__
#pragma mark - Delete Object Methods
#endif

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
	bool success = deserializeDeleteObjectNetworkReply(networkReply, error);

	// Update the ivars
	_isDeleting = false;
	if (success)
		_objectId = "";

	// Clean up
	networkReply->deleteLater();

	return success;
}

bool PFObject::deleteObjectInBackground(QObject *target, const char *action)
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
	if (target)
		QObject::connect(this, SIGNAL(deleteObjectCompleted(bool, PFErrorPtr)), target, action);

	return true;
}

#ifdef __APPLE__
#pragma mark - Delete All Objects Methods
#endif

bool PFObject::deleteAllObjects(PFObjectList objects)
{
	PFErrorPtr error;
	return deleteAllObjects(objects, error);
}

bool PFObject::deleteAllObjects(PFObjectList objects, PFErrorPtr& error)
{
	// Make sure we aren't already deleting any of the objects
	foreach (PFObjectPtr object, objects)
	{
		if (object->_isDeleting)
		{
			qWarning().nospace() << "WARNING: PFObject is already being deleted: " << object->objectId();
			return false;
		}
	}

	// Update the delete state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isDeleting = true;

	// Create a temp object in order to use the create and deserialize methods
	PFObject* callbackObject = new PFObject();

	// Prep the request and data
	QNetworkRequest request;
	QByteArray data;
	callbackObject->createDeleteAllObjectsNetworkRequest(objects, request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->post(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = callbackObject->deserializeDeleteAllObjectsNetworkReply(objects, networkReply, error);

	// Update the delete state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isDeleting = false;

	// Cleanup
	networkReply->deleteLater();
	callbackObject->deleteLater();

	return success;
}

bool PFObject::deleteAllObjectsInBackground(PFObjectList objects, QObject *target, const char *action)
{
	// Make sure we aren't already deleting any of the objects
	foreach (PFObjectPtr object, objects)
	{
		if (object->_isDeleting)
		{
			qWarning().nospace() << "WARNING: PFObject is already being deleted: " << object->objectId();
			return false;
		}
	}

	// Update the delete state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isDeleting = true;

	// Create a temp PFObject on the heap to connect the callbacks which will be cleaned up then
	PFObject* callbackObject = new PFObject();
	gActiveBackgroundObjects.insert(callbackObject, objects);

	// Prep the request and data
	QNetworkRequest request;
	QByteArray data;
	callbackObject->createDeleteAllObjectsNetworkRequest(objects, request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->post(request, data);

	// Hook up the callbacks to the temp object
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), callbackObject, SLOT(handleDeleteAllObjectsCompleted(QNetworkReply*)));
	if (target)
		QObject::connect(callbackObject, SIGNAL(deleteAllObjectsCompleted(bool, PFErrorPtr)), target, action);

	return true;
}

#ifdef __APPLE__
#pragma mark - Data Availibility Methods
#endif

bool PFObject::isDataAvailable()
{
	if (_objectId.isEmpty() || _fetched)
		return true;

	return false;
}

#ifdef __APPLE__
#pragma mark - Fetch Methods
#endif

bool PFObject::fetch()
{
	PFErrorPtr error;
	return fetch(error);
}

bool PFObject::fetch(PFErrorPtr& error)
{
	// Early out if the object is already being fetched
	if (_isFetching)
	{
		qWarning().nospace() << "WARNING: PFObject is already being fetched";
		return false;
	}

	// Cannot fetch an object that hasn't been put into the cloud
	if (_objectId.isEmpty())
	{
		qWarning().nospace() << "WARNING: PFObject cannot be fetched because it has not been saved into the cloud";
		return false;
	}

	// Update the ivar
	_isFetching = true;

	// Create the network request
	QNetworkRequest networkRequest = createFetchNetworkRequest();

	// Execute the network request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = deserializeFetchNetworkReply(networkReply, error);

	// Update the ivars
	_isFetching = false;
	if (success)
		_fetched = true;

	// Clean up
	networkReply->deleteLater();

	return success;
}

bool PFObject::fetchInBackground(QObject *target, const char *action)
{
	// Early out if the object is already being fetched
	if (_isFetching)
	{
		qWarning().nospace() << "WARNING: PFObject is already being fetched";
		return false;
	}

	// Cannot fetch an object that hasn't been put into the cloud
	if (_objectId.isEmpty())
	{
		qWarning().nospace() << "WARNING: PFObject cannot be fetched because it has not been saved into the cloud";
		return false;
	}

	// Update the ivar
	_isFetching = true;

	// Create the network request
	QNetworkRequest networkRequest = createFetchNetworkRequest();

	// Execute the network request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->get(networkRequest);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleFetchCompleted(QNetworkReply*)));
	if (target)
		QObject::connect(this, SIGNAL(fetchCompleted(bool, PFErrorPtr)), target, action);

	return true;
}

#ifdef __APPLE__
#pragma mark - Fetch All Methods
#endif

bool PFObject::fetchAll(PFObjectList objects)
{
	PFErrorPtr error;
	return fetchAll(objects, error);
}

bool PFObject::fetchAll(PFObjectList objects, PFErrorPtr& error)
{
	bool allSucceeded = true;
	foreach (PFObjectPtr object, objects)
	{
		PFErrorPtr fetchError;
		bool success = object->fetch(fetchError);
		if (!success)
		{
			allSucceeded = false;
			error = fetchError;
		}
	}

	return allSucceeded;
}

#ifdef __APPLE__
#pragma mark - Fetch If Needed Methods
#endif

bool PFObject::fetchIfNeeded()
{
	PFErrorPtr error;
	return fetchIfNeeded(error);
}

bool PFObject::fetchIfNeeded(PFErrorPtr& error)
{
	if (isDataAvailable())
		return false;
	else
		return fetch(error);
}

bool PFObject::fetchIfNeededInBackground(QObject *target, const char *action)
{
	if (isDataAvailable())
		return false;
	else
		return fetchInBackground(target, action);
}

#ifdef __APPLE__
#pragma mark - Fetch All If Needed Methods
#endif

bool PFObject::fetchAllIfNeeded(PFObjectList objects)
{
	PFErrorPtr error;
	return fetchAllIfNeeded(objects, error);
}

bool PFObject::fetchAllIfNeeded(PFObjectList objects, PFErrorPtr& error)
{
	bool allSucceeded = true;
	foreach (PFObjectPtr object, objects)
	{
		PFErrorPtr fetchError;
		bool success = object->fetchIfNeeded(fetchError);
		if (!success)
		{
			allSucceeded = false;
			error = fetchError;
		}
	}

	return allSucceeded;
}

#ifdef __APPLE__
#pragma mark - PFSerializable Methods
#endif

QVariant PFObject::fromJson(const QJsonObject& jsonObject)
{
	// Create a new PFObject using the className and objectId
	QString className = jsonObject["className"].toString();
	QString objectId = jsonObject["objectId"].toString();
	PFObjectPtr object = objectWithClassName(className, objectId);

	// Remove some properties from the json object to allow us to recursively convert everything else to our properties
	QJsonObject duplicateJsonObject = jsonObject;
	duplicateJsonObject.remove("__type");
	duplicateJsonObject.remove("className");
	duplicateJsonObject.remove("objectId");

	// Convert the entire json object into a properties variant map and strip out the instance members
	object->_properties = PFConversion::convertJsonToVariant(duplicateJsonObject).toMap();
	object->stripInstanceMembersFromProperties();

	return toVariant(object);
}

bool PFObject::toJson(QJsonObject& jsonObject)
{
	if (_objectId.isEmpty())
	{
		qWarning() << "PFObject::toJson could NOT convert to PFObject to JSON because the _objectId is not set";
		return false;
	}
	else
	{
		jsonObject["__type"] = QString("Pointer");
		jsonObject["className"] = _className;
		jsonObject["objectId"] = _objectId;
		return true;
	}
}

const QString PFObject::pfClassName() const
{
	return "PFObject";
}

#ifdef __APPLE__
#pragma mark - Background Request Completion Signals
#endif

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
		_updatedProperties.clear();

	// Update the ivar
	_isSaving = false;

	// Emit the signal that the save has completed and then disconnect it
	emit saveCompleted(success, error);
	this->disconnect(SIGNAL(saveCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

void PFObject::handleSaveAllCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Fetch the objects out of the active background objects hash table
	PFObjectList objects = gActiveBackgroundObjects.value(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeSaveAllNetworkReply(objects, networkReply, error);

	// Update the save state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isSaving = false;

	// Emit the signal that the save has completed and then disconnect it
	emit saveAllCompleted(success, error);
	this->disconnect(SIGNAL(saveAllCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
	gActiveBackgroundObjects.remove(this);
	this->deleteLater();
}

void PFObject::handleDeleteObjectCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeDeleteObjectNetworkReply(networkReply, error);

	// Update the ivars
	_isDeleting = false;
	if (success)
		_objectId = "";

	// Emit the signal that the delete object has completed and then disconnect it
	emit deleteObjectCompleted(success, error);
	this->disconnect(SIGNAL(deleteObjectCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

void PFObject::handleDeleteAllObjectsCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Fetch the objects out of the active background objects hash table
	PFObjectList objects = gActiveBackgroundObjects.value(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeDeleteAllObjectsNetworkReply(objects, networkReply, error);

	// Update the delete state for all objects
	foreach (PFObjectPtr object, objects)
		object->_isDeleting = false;

	// Emit the signal that the delete all objects has completed and then disconnect it
	emit deleteAllObjectsCompleted(success, error);
	this->disconnect(SIGNAL(deleteAllObjectsCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
	gActiveBackgroundObjects.remove(this);
	this->deleteLater();
}

void PFObject::handleFetchCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeFetchNetworkReply(networkReply, error);

	// Update the ivars
	_isFetching = false;
	if (success)
		_fetched = true;

	// Emit the signal that the delete object has completed and then disconnect it
	emit fetchCompleted(success, error);
	this->disconnect(SIGNAL(fetchCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

#ifdef __APPLE__
#pragma mark - Protected Methods
#endif

bool PFObject::needsUpdate()
{
	return !_objectId.isEmpty();
}

#ifdef __APPLE__
#pragma mark - Network Request Builder Methods
#endif

void PFObject::createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data)
{
	// Create the url based on whether we should create or update the PFObject
	bool updateRequired = needsUpdate();
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className);
	if (updateRequired)
		url = QUrl(url.toString() + "/" + _objectId);

	// Create a network request
	request = QNetworkRequest(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	// Figure out whether we need to
	QVariantMap objectsToSerialize;
	if (updateRequired)
		objectsToSerialize = _updatedProperties;
	else
		objectsToSerialize = _properties;

	// Serialize all the objects into json
	QJsonObject jsonObject;
	foreach (const QString& key, objectsToSerialize.keys())
	{
		QVariant objectToSerialize = objectsToSerialize[key];
		jsonObject[key] = PFConversion::convertVariantToJson(objectToSerialize);
	}
	data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}

void PFObject::createSaveAllNetworkRequest(PFObjectList objects, QNetworkRequest& request, QByteArray& data)
{
	// Create a network request
	request = QNetworkRequest(QString("https://api.parse.com/1/batch"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	// Iterate through all the objects and create the json for each one
	QJsonArray jsonRequestArray;
	foreach (PFObjectPtr object, objects)
	{
		// Figure out whether we need to update the object or save it
		bool updateRequired = object->needsUpdate();
		QVariantMap objectsToSerialize;
		if (updateRequired)
			objectsToSerialize = object->_updatedProperties;
		else
			objectsToSerialize = object->_properties;

		// Serialize all the objects into json
		QJsonObject jsonObjectBody;
		foreach (const QString& key, objectsToSerialize.keys())
		{
			QVariant objectToSerialize = objectsToSerialize[key];
			jsonObjectBody[key] = PFConversion::convertVariantToJson(objectToSerialize);
		}

		// Create the json request
		QJsonObject jsonRequest;
		if (updateRequired)
		{
			jsonRequest["method"] = QString("PUT");
			jsonRequest["path"] = QString("/1/classes/") + object->className() + "/" + object->objectId();
		}
		else
		{
			jsonRequest["method"] = QString("POST");
			jsonRequest["path"] = QString("/1/classes/") + object->className();
		}
		jsonRequest["body"] = jsonObjectBody;

		// Add the json request to the array
		jsonRequestArray.append(jsonRequest);
	}

	// Create the final json data
	QJsonObject finalJsonRequest;
	finalJsonRequest["requests"] = jsonRequestArray;
	data = QJsonDocument(finalJsonRequest).toJson(QJsonDocument::Compact);
}

QNetworkRequest PFObject::createDeleteObjectNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className + "/" + _objectId);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

void PFObject::createDeleteAllObjectsNetworkRequest(PFObjectList objects, QNetworkRequest& request, QByteArray& data)
{
	// Create a network request
	request = QNetworkRequest(QString("https://api.parse.com/1/batch"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	// Iterate through all the objects and create the json for each one
	QJsonArray jsonRequestArray;
	foreach (PFObjectPtr object, objects)
	{
		// Create the json request
		QJsonObject jsonRequest;
		jsonRequest["method"] = QString("DELETE");
		jsonRequest["path"] = QString("/1/classes/") + object->className() + "/" + object->objectId();

		// Add the json request to the array
		jsonRequestArray.append(jsonRequest);
	}

	// Create the final json data
	QJsonObject finalJsonRequest;
	finalJsonRequest["requests"] = jsonRequestArray;
	data = QJsonDocument(finalJsonRequest).toJson(QJsonDocument::Compact);
}

QNetworkRequest PFObject::createFetchNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className + "/" + _objectId);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

#ifdef __APPLE__
#pragma mark - Network Reply Deserialization Methods
#endif

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
			qDebug().nospace() << "Created Object:" << _className << " with objectId:" << _objectId;
		}
		else // UPDATED
		{
			QString updatedAt = jsonObject["updatedAt"].toString();
			_updatedAt = PFDateTime::dateTimeFromParseString(updatedAt);
			qDebug().nospace() << "Updated Object:" << _className << " with objectId:" << _objectId;
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

bool PFObject::deserializeSaveAllNetworkReply(PFObjectList objects, QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the doc as a json array
		QJsonArray jsonArray = doc.array();

		// Go through each item in the reply and update the respective object
		bool allSucceeded = true;
		unsigned int counter = 0;
		foreach (const QJsonValue& jsonValue, jsonArray)
		{
			// Grab the matching object and whether it was updated or saved
			PFObjectPtr object = objects.at(counter);
			bool updated = object->needsUpdate();

			// Handle whether the save was a success or error
			QJsonObject jsonObject = jsonValue.toObject();
			if (jsonObject.contains("success"))
			{
				// Need to make sure we parse the json properly depending on whether the object was saved or updated
				jsonObject = jsonObject["success"].toObject();
				if (updated) // UPDATED
				{
					// Extract the updatedAt property
					QString updatedAt = jsonObject["updatedAt"].toString();
					object->_updatedAt = PFDateTime::dateTimeFromParseString(updatedAt);
					qDebug().nospace() << "Updated Object:" << object->_className << " with objectId:" << object->_objectId;

					// Clear out the updated properties
					object->_updatedProperties.clear();
				}
				else // SAVED
				{
					QString createdAt = jsonObject["createdAt"].toString();
					object->_createdAt = PFDateTime::dateTimeFromParseString(createdAt);
					object->_objectId = jsonObject["objectId"].toString();
					qDebug().nospace() << "Created Object:" << object->_className << " with objectId:" << object->_objectId;
				}
			}
			else
			{
				// The PFError structure doesn't support more than a single error at a time. Therefore, we're just
				// going to keep stomping the error with the latest one if we have multiple failures.
				jsonObject = jsonObject["error"].toObject();
				int errorCode = jsonObject["code"].toInt();
				QString errorMessage = jsonObject["error"].toString();
				error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
				allSucceeded = false;
			}

			++counter;
		}

		return allSucceeded;
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

bool PFObject::deserializeDeleteObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
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

bool PFObject::deserializeDeleteAllObjectsNetworkReply(PFObjectList objects, QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the doc as a json array
		QJsonArray jsonArray = doc.array();

		// Go through each item in the reply and update the respective object
		bool allSucceeded = true;
		unsigned int counter = 0;
		foreach (const QJsonValue& jsonValue, jsonArray)
		{
			// Grab the matching object
			PFObjectPtr object = objects.at(counter);

			// Handle whether the save was a success or error
			QJsonObject jsonObject = jsonValue.toObject();
			if (jsonObject.contains("success"))
			{
				// Reset the object id
				object->_objectId = "";
			}
			else
			{
				// The PFError structure doesn't support more than a single error at a time. Therefore, we're just
				// going to keep stomping the error with the latest one if we have multiple failures.
				jsonObject = jsonObject["error"].toObject();
				int errorCode = jsonObject["code"].toInt();
				QString errorMessage = jsonObject["error"].toString();
				error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
				allSucceeded = false;
			}

			++counter;
		}

		return allSucceeded;
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

bool PFObject::deserializeFetchNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Deserialize the json into our properties variant map and strip out the instance members
		_properties = PFConversion::convertJsonToVariant(jsonObject).toMap();
		stripInstanceMembersFromProperties();

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

#ifdef __APPLE__
#pragma mark - Instance Member Property Stripping Methods
#endif

void PFObject::stripInstanceMembersFromProperties()
{
	// className
	if (_properties.contains("className"))
		_className = _properties.take("className").toString();

	// objectId
	if (_properties.contains("objectId"))
		_objectId = _properties.take("objectId").toString();

	// createdAt
	if (_properties.contains("createdAt"))
	{
		QString createdAt = _properties.take("createdAt").toString();
		_createdAt = PFDateTime::dateTimeFromParseString(createdAt);
	}

	// updatedAt
	if (_properties.contains("updatedAt"))
	{
		QString updatedAt = _properties.take("updatedAt").toString();
		_updatedAt = PFDateTime::dateTimeFromParseString(updatedAt);
	}

	// ACL
	if (_properties.contains("ACL"))
	{
		QVariantMap aclVariantMap = _properties.take("ACL").toMap();
		QJsonObject aclJsonObject = QJsonObject::fromVariantMap(aclVariantMap);
		QVariant aclVariant = PFACL::fromJson(aclJsonObject);
		PFACLPtr acl = PFACL::ACLFromVariant(aclVariant);
		setACL(acl);
	}
}

}	// End of parse namespace
