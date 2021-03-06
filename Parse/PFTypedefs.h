//
//  PFTypedefs.h
//  Parse
//
//  Created by Christian Noon on 11/13/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFTYPEDEFS_H
#define PARSE_PFTYPEDEFS_H

// Qt headers
#include <QList>
#include <QSharedPointer>

namespace parse {

// Forward declarations
class PFACL;
class PFDateTime;
class PFError;
class PFFile;
class PFObject;
class PFQuery;
class PFSerializable;
class PFUser;

// Parse Typedefs
typedef QSharedPointer<PFACL> PFACLPtr;
typedef QSharedPointer<PFDateTime> PFDateTimePtr;
typedef QSharedPointer<PFError> PFErrorPtr;
typedef QSharedPointer<PFFile> PFFilePtr;
typedef QSharedPointer<PFObject> PFObjectPtr;
typedef QSharedPointer<PFQuery> PFQueryPtr;
typedef QSharedPointer<PFSerializable> PFSerializablePtr;
typedef QSharedPointer<PFUser> PFUserPtr;

// Parse Collection Typedefs
typedef QList<PFObjectPtr> PFObjectList;

// Qt Typedefs
typedef QSharedPointer<QByteArray> QByteArrayPtr;

}	// End of parse namespace

#endif	// End of PARSE_PFTYPEDEFS_H
