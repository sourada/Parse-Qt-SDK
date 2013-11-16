//
//  PFObject.h
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
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

	// ACL Accessor Methods
	void setACL(PFACLPtr acl);
	PFACLPtr ACL();

	// Object Info Getter Methods
	virtual const QString className();
	const QString& objectId();
	PFDateTimePtr createdAt();
	PFDateTimePtr updatedAt();

	// Save Methods - saveCompleteAction signature: (bool succeeded, PFErrorPtr error)
	bool save();
	bool save(PFErrorPtr& error);
	bool saveInBackground(QObject *saveCompleteTarget, const char *saveCompleteAction);

	// Delete Methods - deleteObjectCompleteAction signature: (bool succeeded, PFErrorPtr error)
	bool deleteObject();
	bool deleteObject(PFErrorPtr& error);
	bool deleteObjectInBackground(QObject *deleteObjectCompleteTarget, const char *deleteObjectCompleteAction);

	// Returns true if new or has been fetched, false otherwise
	bool isDataAvailable();

	// Fetch Methods - fetchCompleteAction signature: (bool succeeded, PFErrorPtr error)
	bool fetch();
	bool fetch(PFErrorPtr& error);
	bool fetchInBackground(QObject *fetchCompleteTarget, const char *fetchCompleteAction);

	// Fetch If Needed Methods - fetchCompleteAction signature: (bool succeeded, PFErrorPtr error)
	// Returns true if a fetch was actually started, false otherwise. Also, these methods only
	// execute a fetch from the server if isDataAvailable() is false.
	bool fetchIfNeeded();
	bool fetchIfNeeded(PFErrorPtr& error);
	bool fetchIfNeededInBackground(QObject *fetchCompleteTarget, const char *fetchCompleteAction);

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
	void handleDeleteObjectCompleted(QNetworkReply* networkReply);
	void handleFetchCompleted(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void saveCompleted(bool succeeded, PFErrorPtr error);
	void deleteObjectCompleted(bool succeeded, PFErrorPtr error);
	void fetchCompleted(bool succeeded, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFObject();
	~PFObject();

	// Returns true if the if the object exists in the cloud and needs an update,
	// false if it hasn't been put into the cloud yet
	bool needsUpdate();

	// Network Request Builder Methods
	virtual void createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data);
	virtual QNetworkRequest createDeleteObjectNetworkRequest();
	virtual QNetworkRequest createFetchNetworkRequest();

	// Network Reply Deserialization Methods
	bool deserializeSaveNetworkReply(QNetworkReply* networkReply, bool updated, PFErrorPtr& error);
	bool deserializeDeleteObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	virtual bool deserializeFetchNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Recursive JSON Conversion Helper Methods
	QJsonValue convertDataToJson(const QVariant& data);
	QVariant convertJsonToVariant(const QJsonValue& jsonValue);
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
