//
//  PFObject.h
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFOBJECT_H
#define PARSE_PFOBJECT_H

// Parse headers
#include "PFSerializable.h"

// Qt headers
#include <QHash>
#include <QNetworkReply>

namespace parse {

class PFObject : public PFSerializable
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creation Methods
	static PFObjectPtr objectWithClassName(const QString& className);
	static PFObjectPtr objectWithClassName(const QString& className, const QString& objectId);
	static PFObjectPtr objectFromVariant(const QVariant& variant);

	// Object Storage Methods
	void setObjectForKey(const QVariant& object, const QString& key);
	void setObjectForKey(PFSerializablePtr object, const QString& key);
	bool removeObjectForKey(const QString& key);
	QVariant objectForKey(const QString& key);
	QStringList allKeys();

	// Increment Methods
	void incrementKey(const QString& key);
	void incrementKeyByAmount(const QString& key, int amount);

	// List Add & Remove Methods
	void addObjectToListForKey(const QVariant& object, const QString& key);
	void addObjectToListForKey(PFSerializablePtr object, const QString& key);
	void addObjectsToListForKey(const QVariantList& objects, const QString& key);
	void addUniqueObjectToListForKey(const QVariant& object, const QString& key);
	void addUniqueObjectToListForKey(PFSerializablePtr object, const QString& key);
	void addUniqueObjectsToListForKey(const QVariantList& objects, const QString& key);
	void removeObjectFromListForKey(const QVariant& object, const QString& key);
	void removeObjectFromListForKey(PFSerializablePtr object, const QString& key);
	void removeObjectsFromListForKey(const QVariantList& objects, const QString& key);

	// ACL Accessor Methods
	void setACL(PFACLPtr acl);
	PFACLPtr ACL();

	// Object Info Getter Methods
	virtual const QString className();
	const QString& objectId();
	PFDateTimePtr createdAt();
	PFDateTimePtr updatedAt();

	// Save Methods - action signature: (bool succeeded, PFErrorPtr error)
	bool save();
	bool save(PFErrorPtr& error);
	bool saveInBackground(QObject *target = 0, const char *action = 0);

	// Save All Methods - action signature: (bool succeeded, PFErrorPtr error)
	static bool saveAll(PFObjectList objects);
	static bool saveAll(PFObjectList objects, PFErrorPtr& error);
	static bool saveAllInBackground(PFObjectList objects, QObject *target, const char *action);

	// Delete Methods - action signature: (bool succeeded, PFErrorPtr error)
	bool deleteObject();
	bool deleteObject(PFErrorPtr& error);
	bool deleteObjectInBackground(QObject *target = 0, const char *action = 0);

	// Delete All Methods - action signature: (bool succeeded, PFErrorPtr error)
	static bool deleteAllObjects(PFObjectList objects);
	static bool deleteAllObjects(PFObjectList objects, PFErrorPtr& error);
	static bool deleteAllObjectsInBackground(PFObjectList objects, QObject *target = 0, const char *action = 0);

	// Returns true if new or has been fetched, false otherwise
	bool isDataAvailable();

	// Fetch Methods - action signature: (bool succeeded, PFErrorPtr error)
	bool fetch();
	bool fetch(PFErrorPtr& error);
	bool fetchInBackground(QObject *target = 0, const char *action = 0);

	// Fetch All Methods
	static bool fetchAll(PFObjectList objects);
	static bool fetchAll(PFObjectList objects, PFErrorPtr& error);

	// Fetch If Needed Methods - action signature: (bool succeeded, PFErrorPtr error)
	// Returns true if a fetch was actually started, false otherwise. Also, these methods only
	// execute a fetch from the server if isDataAvailable() is false.
	bool fetchIfNeeded();
	bool fetchIfNeeded(PFErrorPtr& error);
	bool fetchIfNeededInBackground(QObject *target = 0, const char *action = 0);

	// Fetch All Methods
	static bool fetchAllIfNeeded(PFObjectList objects);
	static bool fetchAllIfNeeded(PFObjectList objects, PFErrorPtr& error);

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	static QVariant fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString pfClassName() const;

protected slots:

	// Background Network Reply Completion Slots
	void handleSaveCompleted(QNetworkReply* networkReply);
	void handleSaveAllCompleted(QNetworkReply* networkReply);
	void handleDeleteObjectCompleted(QNetworkReply* networkReply);
	void handleDeleteAllObjectsCompleted(QNetworkReply* networkReply);
	void handleFetchCompleted(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void saveCompleted(bool succeeded, PFErrorPtr error);
	void saveAllCompleted(bool succeeded, PFErrorPtr error);
	void deleteObjectCompleted(bool succeeded, PFErrorPtr error);
	void deleteAllObjectsCompleted(bool succeeded, PFErrorPtr error);
	void fetchCompleted(bool succeeded, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFObject();
	~PFObject();

	// Returns true if the if the object exists in the cloud and needs an update,
	// false if it hasn't been put into the cloud yet
	bool needsUpdate();

	// Network Request Builder Methods
	void createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data);
	void createSaveAllNetworkRequest(PFObjectList objects, QNetworkRequest& request, QByteArray& data);
	QNetworkRequest createDeleteObjectNetworkRequest();
	void createDeleteAllObjectsNetworkRequest(PFObjectList objects, QNetworkRequest& request, QByteArray& data);
	QNetworkRequest createFetchNetworkRequest();

	// Network Reply Deserialization Methods
	bool deserializeSaveNetworkReply(QNetworkReply* networkReply, bool updated, PFErrorPtr& error);
	bool deserializeSaveAllNetworkReply(PFObjectList objects, QNetworkReply* networkReply, PFErrorPtr& error);
	bool deserializeDeleteObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	bool deserializeDeleteAllObjectsNetworkReply(PFObjectList objects, QNetworkReply* networkReply, PFErrorPtr& error);
	virtual bool deserializeFetchNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Strips the instance members from the properties after recursive fetching
	virtual void stripInstanceMembersFromProperties();

	// Instance members
	QString				_className;
	QString				_objectId;
	PFACLPtr			_acl;
	PFDateTimePtr		_createdAt;
	PFDateTimePtr		_updatedAt;
	QVariantMap			_properties;
	QVariantMap			_updatedProperties;
	bool				_isSaving;
	bool				_isDeleting;
	bool				_isFetching;
	bool				_fetched;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFObjectPtr)

#endif	// End of PARSE_PFOBJECT_H
