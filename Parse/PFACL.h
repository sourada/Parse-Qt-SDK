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
#include "PFUser.h"

// Qt headers
#include <QObject>

namespace parse {

// Typedefs
class PFACL;
typedef QSharedPointer<PFACL> PFACLPtr;

/**
 * It is important to note that the properties will not store a false value. Instead
 * the key is removed from the properties. The Parse REST API does not allow false
 * values to be passed for the ACL. For more details please refer to the implementation.
 */
class PFACL : public QObject
{
public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	/** Creates an ACL with no permissions granted. */
	static PFACLPtr ACL();

	/** Creates an ACL where only the provided user has access. */
	static PFACLPtr ACLWithUser(PFUserPtr user);

	/** Sets a default ACL that will be applied to all PFObjects when they are created. */
	static void setDefaultACLWithAccessForCurrentUser(PFACLPtr defaultACL, bool currentUserAccess);

	/** Helper method for creating a QVariant with a PFACLPtr object. */
	static QVariant variantWithACL(const PFACLPtr& acl);

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
	void setReadAccessForUser(bool allowed, PFUserPtr user);
	void setWriteAccessForUser(bool allowed, PFUserPtr user);
	bool readAccessForUser(PFUserPtr user);
	bool writeAccessForUser(PFUserPtr user);

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	/** Destructor. */
	virtual ~PFACL();

	// PFSerializable Methods
	void fromJson(const QJsonObject& jsonObject);
	void toJson(QJsonObject& jsonObject);

protected:

	/** Constructor. */
	PFACL();

	/** Direct access to the default acl properties for the PFObject class. */
	friend class PFObject;
	static void defaultACLWithCurrentUserAccess(PFACLPtr& defaultACL, bool& currentUserAccess);
	PFACLPtr clone();

	/** Instance members. */
	QVariantMap		_properties;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFACLPtr)

#endif	// End of PARSE_PFACL_H
