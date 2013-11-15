//
//  PFACL.h
//  Parse
//
//  Created by Christian Noon on 11/8/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFACL_H
#define PARSE_PFACL_H

// Parse headers
#include "PFSerializable.h"

// Qt headers
#include <QObject>

namespace parse {

// It is important to note that the properties will not store a false value. Instead
// the key is removed from the properties. The Parse REST API does not allow false
// values to be passed for the ACL. For more details please refer to the implementation.
class PFACL : public PFSerializable
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creates an ACL with no permissions granted
	static PFACLPtr ACL();
	static PFACLPtr ACLFromVariant(const QVariant& variant);

	// Creates an ACL where only the provided user has access
	static PFACLPtr ACLWithUser(PFUserPtr user);

	// Sets a default ACL that will be applied to all PFObjects when they are created
	static void setDefaultACLWithAccessForCurrentUser(PFACLPtr defaultACL, bool currentUserAccess);

	// Public Access Methods
	void setPublicReadAccess(bool allowed);
	void setPublicWriteAccess(bool allowed);
	bool publicReadAccess();
	bool publicWriteAccess();

	// Per-User Access Methods
	void setReadAccessForUserId(bool allowed, const QString& userId);
	void setWriteAccessForUserId(bool allowed, const QString& userId);
	bool readAccessForUserId(const QString& userId);
	bool writeAccessForUserId(const QString& userId);

	// Explicit Per-User Access Methods (does not consider the public read/write access)
	bool setReadAccessForUser(bool allowed, PFUserPtr user);
	bool setWriteAccessForUser(bool allowed, PFUserPtr user);
	bool readAccessForUser(PFUserPtr user);
	bool writeAccessForUser(PFUserPtr user);

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	static QVariant fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString className() const;

protected:

	// Constructor / Destructor
	PFACL();
	virtual ~PFACL();

	// Direct access to the default acl properties for the PFObject class
	friend class PFObject;
	static void defaultACLWithCurrentUserAccess(PFACLPtr& defaultACL, bool& currentUserAccess);
	PFACLPtr clone();

	// Instance members
	QVariantMap _properties;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFACLPtr)

#endif	// End of PARSE_PFACL_H
