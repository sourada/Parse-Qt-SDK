//
//  PFACL.cpp
//  Parse
//
//  Created by Christian Noon on 11/8/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include "PFACL.h"
#include "PFUser.h"

// Qt headers
#include <QJsonDocument>

#define PUBLIC_ACCESS_IDENTIFIER		"*"

namespace parse {

// Static Globals
static PFACLPtr gDefaultACL = PFACLPtr();
static bool gDefaultCurrentUserAccess = false;

#pragma mark - Memory Management Methods

PFACL::PFACL()
{
	qDebug().nospace() << "Created PFACL(" << QString().sprintf("%8p", this) << ")";
}

PFACL::~PFACL()
{
	qDebug().nospace() << "Destroyed PFACL(" << QString().sprintf("%8p", this) << ")";
}

#pragma mark - Creation Methods

PFACLPtr PFACL::ACL()
{
	return PFACLPtr(new PFACL(), &QObject::deleteLater);
}

PFACLPtr PFACL::ACLFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = PFSerializable::fromVariant(variant);
	if (!serializable.isNull())
		return serializable.objectCast<PFACL>();

	return PFACLPtr();
}

PFACLPtr PFACL::ACLWithUser(PFUserPtr user)
{
	if (user.isNull())
	{
		qWarning() << "PFACL::ACLWithUser failed since the user was NULL";
		return PFACLPtr();
	}
	else if (user->objectId().isEmpty())
	{
		qWarning() << "PFACL::ACLWithUser failed since the user does not have an object id";
		return PFACLPtr();
	}
	else
	{
		PFACLPtr acl = PFACL::ACL();
		acl->setReadAccessForUser(true, user);
		acl->setWriteAccessForUser(true, user);
		return acl;
	}
}

PFACLPtr PFACL::clone()
{
	PFACLPtr acl = PFACL::ACL();
	acl->_properties = _properties;
	return acl;
}

#pragma mark - Default ACL Methods

void PFACL::setDefaultACLWithAccessForCurrentUser(PFACLPtr defaultACL, bool currentUserAccess)
{
	gDefaultACL = defaultACL;
	gDefaultCurrentUserAccess = currentUserAccess;
}

void PFACL::defaultACLWithCurrentUserAccess(PFACLPtr& defaultACL, bool& currentUserAccess)
{
	defaultACL = gDefaultACL;
	currentUserAccess = gDefaultCurrentUserAccess;
}

#pragma mark - Public Access Methods

void PFACL::setPublicReadAccess(bool allowed)
{
	// Get the public properties if they already exist
	QVariantMap publicProperties;
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
		publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();

	// Modify the "read" property
	if (allowed)
		publicProperties["read"] = true;
	else
		publicProperties.remove("read");

	// Replace the public access map or remove it if it's empty
	if (publicProperties.isEmpty())
		_properties.remove(PUBLIC_ACCESS_IDENTIFIER);
	else
		_properties[PUBLIC_ACCESS_IDENTIFIER] = publicProperties;
}

void PFACL::setPublicWriteAccess(bool allowed)
{
	// Get the public properties if they already exist
	QVariantMap publicProperties;
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
		publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();

	// Modify the "write" property
	if (allowed)
		publicProperties["write"] = true;
	else
		publicProperties.remove("write");

	// Replace the public access map or remove it if it's empty
	if (publicProperties.isEmpty())
		_properties.remove(PUBLIC_ACCESS_IDENTIFIER);
	else
		_properties[PUBLIC_ACCESS_IDENTIFIER] = publicProperties;
}

bool PFACL::publicReadAccess()
{
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
	{
		QVariantMap publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();
		return publicProperties.contains("read");
	}

	return false;
}

bool PFACL::publicWriteAccess()
{
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
	{
		QVariantMap publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();
		return publicProperties.contains("write");
	}

	return false;
}

#pragma mark - Per-User Access Methods

void PFACL::setReadAccessForUserId(bool allowed, const QString& userId)
{
	// Get the user properties if they already exist
	QVariantMap userProperties;
	if (_properties.contains(userId))
		userProperties = _properties[userId].toMap();

	// Modify the "read" property
	if (allowed)
		userProperties["read"] = true;
	else
		userProperties.remove("read");

	// Replace the old user access map with the new one
	if (userProperties.isEmpty())
		_properties.remove(userId);
	else
		_properties[userId] = userProperties;
}

void PFACL::setWriteAccessForUserId(bool allowed, const QString& userId)
{
	// Get the user properties if they already exist
	QVariantMap userProperties;
	if (_properties.contains(userId))
		userProperties = _properties[userId].toMap();

	// Modify the "write" property
	if (allowed)
		userProperties["write"] = true;
	else
		userProperties.remove("write");

	// Replace the old user access map with the new one
	if (userProperties.isEmpty())
		_properties.remove(userId);
	else
		_properties[userId] = userProperties;
}

bool PFACL::readAccessForUserId(const QString& userId)
{
	// User Properties
	bool isUserReadable = false;
	if (_properties.contains(userId)) // User Properties
	{
		QVariantMap userProperties = _properties[userId].toMap();
		isUserReadable = userProperties.contains("read");
	}

	// Public Properties
	bool isPublicReadable = false;
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
	{
		QVariantMap publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();
		isPublicReadable = publicProperties.contains("read");
	}

	return (isUserReadable || isPublicReadable);
}

bool PFACL::writeAccessForUserId(const QString& userId)
{
	// User Properties
	bool isUserWritable = false;
	if (_properties.contains(userId)) // User Properties
	{
		QVariantMap userProperties = _properties[userId].toMap();
		isUserWritable = userProperties.contains("write");
	}

	// Public Properties
	bool isPublicWritable = false;
	if (_properties.contains(PUBLIC_ACCESS_IDENTIFIER))
	{
		QVariantMap publicProperties = _properties[PUBLIC_ACCESS_IDENTIFIER].toMap();
		isPublicWritable = publicProperties.contains("write");
	}

	return (isUserWritable || isPublicWritable);
}

#pragma mark - Explicit Per-User Access Methods

bool PFACL::setReadAccessForUser(bool allowed, PFUserPtr user)
{
	if (user.isNull())
	{
		qWarning() << "PFACL::setReadAccessForUser failed because the user was NULL";
		return false;
	}
	else if (user->objectId().isEmpty())
	{
		qWarning() << "PFACL::setReadAccessForUser failed because the user did not have an object id";
		return false;
	}
	else
	{
		setReadAccessForUserId(allowed, user->objectId());
		return true;
	}
}

bool PFACL::setWriteAccessForUser(bool allowed, PFUserPtr user)
{
	if (user.isNull())
	{
		qWarning() << "PFACL::setWriteAccessForUser failed because the user was NULL";
		return false;
	}
	else if (user->objectId().isEmpty())
	{
		qWarning() << "PFACL::setWriteAccessForUser failed because the user did not have an object id";
		return false;
	}
	else
	{
		setWriteAccessForUserId(allowed, user->objectId());
		return true;
	}
}

bool PFACL::readAccessForUser(PFUserPtr user)
{
	QString userId = user->objectId();
	if (_properties.contains(userId))
	{
		QVariantMap userProperties = _properties[userId].toMap();
		return userProperties.contains("read");
	}

	return false;
}

bool PFACL::writeAccessForUser(PFUserPtr user)
{
	QString userId = user->objectId();
	if (_properties.contains(userId))
	{
		QVariantMap userProperties = _properties[userId].toMap();
		return userProperties.contains("write");
	}

	return false;
}

#pragma mark - PFSerializable Methods

QVariant PFACL::fromJson(const QJsonObject &jsonObject)
{
	PFACLPtr ACL = PFACL::ACL();
	ACL->_properties = jsonObject.toVariantMap();

	return PFSerializable::toVariant(ACL);
}

bool PFACL::toJson(QJsonObject& jsonObject)
{
	jsonObject = QJsonObject::fromVariantMap(_properties);
	return true;
}

const QString PFACL::className() const
{
	return "PFACL";
}

}	// End of parse namespace
