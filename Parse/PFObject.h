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
	const QVariant& objectForKey(const QString& key);
	QStringList allKeys();

	// ACL Accessor Methods
	void setACL(PFACLPtr acl);
	PFACLPtr ACL();

	// Object Info Getter Methods
	virtual const QString parseClassName();
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

	////////////////////////////////////////////////////////////////
	//
	// TODO List (required):
	// - removeObjectForKey
	// - Array Add and Remove
	// - Increment Key
	// - Fetch, Fetch If Needed
	//
	// TODO List (optional):
	// - Save All
	// - Delete All
	// - Fetch All, Fetch All If Needed
	//
	////////////////////////////////////////////////////////////////

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	virtual PFSerializablePtr fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString className() const;

protected slots:

	// Background Network Reply Completion Slots
	void handleSaveCompleted(QNetworkReply* networkReply);
	void handleDeleteObjectCompleted(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void saveCompleted(bool succeeded, PFErrorPtr error);
	void deleteObjectCompleted(bool succeeded, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFObject();
	~PFObject();

	// Returns true if the if the object exists in the cloud and needs an update,
	// false if it hasn't been put into the cloud yet
	bool needsUpdate();

	// Network Request Builder Methods
	virtual void createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data);
	QJsonValue convertDataToJson(const QVariant& data); // Recursive
	virtual QNetworkRequest createDeleteObjectNetworkRequest();

	// Network Reply Deserialization Methods
	bool deserializeSaveNetworkReply(QNetworkReply* networkReply, bool updated, PFErrorPtr& error);
	bool deserializeDeleteObjectReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Instance members
	QString				_parseClassName;
	QString				_objectId;
	PFACLPtr			_acl;
	PFDateTimePtr		_createdAt;
	PFDateTimePtr		_updatedAt;
	QVariantMap			_childObjects;
	QVariantMap			_updatedChildObjects;
	bool				_isSaving;
	bool				_isDeleting;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFObjectPtr)

#endif	// End of PARSE_PFOBJECT_H
