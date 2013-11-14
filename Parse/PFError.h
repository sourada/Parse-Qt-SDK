//
//  PFError.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFERROR_H
#define PARSE_PFERROR_H

// Parse headers
#include "PFTypedefs.h"

// Qt headers
#include <QObject>
#include <QSharedPointer>
#include <QString>

/** Error 1: Internal server error. No information available. */
extern int const kPFErrorInternalServer;

/** Error 100: The connection to the Parse servers failed. */
extern int const kPFErrorConnectionFailed;
/** Error 101: Object doesn't exist, or has an incorrect password. */
extern int const kPFErrorObjectNotFound;
/** Error 102: You tried to find values matching a datatype that doesn't support exact database matching, like an array or a dictionary. */
extern int const kPFErrorInvalidQuery;
/** Error 103: Missing or invalid classname. Classnames are case-sensitive. They must start with a letter, and a-zA-Z0-9_ are the only valid characters. */
extern int const kPFErrorInvalidClassName;
/** Error 104: Missing object id. */
extern int const kPFErrorMissingObjectId;
/** Error 105: Invalid key name. Keys are case-sensitive. They must start with a letter, and a-zA-Z0-9_ are the only valid characters. */
extern int const kPFErrorInvalidKeyName;
/** Error 106: Malformed pointer. Pointers must be arrays of a classname and an object id. */
extern int const kPFErrorInvalidPointer;
/** Error 107: Malformed json object. A json dictionary is expected. */
extern int const kPFErrorInvalidJSON;
/** Error 108: Tried to access a feature only available internally. */
extern int const kPFErrorCommandUnavailable;
/** Error 111: Field set to incorrect type. */
extern int const kPFErrorIncorrectType;
/** Error 112: Invalid channel name. A channel name is either an empty string (the broadcast channel) or contains only a-zA-Z0-9_ characters and starts with a letter. */
extern int const kPFErrorInvalidChannelName;
/** Error 114: Invalid device token. */
extern int const kPFErrorInvalidDeviceToken;
/** Error 115: Push is misconfigured. See details to find out how. */
extern int const kPFErrorPushMisconfigured;
/** Error 116: The object is too large. */
extern int const kPFErrorObjectTooLarge;
/** Error 119: That operation isn't allowed for clients. */
extern int const kPFErrorOperationForbidden;
/** Error 120: The results were not found in the cache. */
extern int const kPFErrorCacheMiss;
/** Error 121: Keys in NSDictionary values may not include '$' or '.'. */
extern int const kPFErrorInvalidNestedKey;
/** Error 122: Invalid file name. A file name contains only a-zA-Z0-9_. characters and is between 1 and 36 characters. */
extern int const kPFErrorInvalidFileName;
/** Error 123: Invalid ACL. An ACL with an invalid format was saved. This should not happen if you use PFACL. */
extern int const kPFErrorInvalidACL;
/** Error 124: The request timed out on the server. Typically this indicates the request is too expensive. */
extern int const kPFErrorTimeout;
/** Error 125: The email address was invalid. */
extern int const kPFErrorInvalidEmailAddress;
/** Error 137: A unique field was given a value that is already taken. */
extern int const kPFErrorDuplicateValue;
/** Error 139: Role's name is invalid. */
extern int const kPFErrorInvalidRoleName;
/** Error 140: Exceeded an application quota.  Upgrade to resolve. */
extern int const kPFErrorExceededQuota;
/** Error 141: Cloud Code script had an error. */
extern int const kPFScriptError;
/** Error 142: Cloud Code validation failed. */
extern int const kPFValidationError;
/** Error 143: Product purchase receipt is missing */
extern int const kPFErrorReceiptMissing;
/** Error 144: Product purchase receipt is invalid */
extern int const kPFErrorInvalidPurchaseReceipt;
/** Error 145: Payment is disabled on this device */
extern int const kPFErrorPaymentDisabled;
/** Error 146: The product identifier is invalid */
extern int const kPFErrorInvalidProductIdentifier;
/** Error 147: The product is not found in the App Store */
extern int const kPFErrorProductNotFoundInAppStore;
/** Error 148: The Apple server response is not valid */
extern int const kPFErrorInvalidServerResponse;
/** Error 149: Product fails to download due to file system error */
extern int const kPFErrorProductDownloadFileSystemFailure;
/** Error 150: Fail to convert data to image. */
extern int const kPFErrorInvalidImageData;
/** Error 151: Unsaved file. */
extern int const kPFErrorUnsavedFile;
/** Error 153: Fail to delete file. */
extern int const kPFErrorFileDeleteFailure;

/** Error 200: Username is missing or empty */
extern int const kPFErrorUsernameMissing;
/** Error 201: Password is missing or empty */
extern int const kPFErrorUserPasswordMissing;
/** Error 202: Username has already been taken */
extern int const kPFErrorUsernameTaken;
/** Error 203: Email has already been taken */
extern int const kPFErrorUserEmailTaken;
/** Error 204: The email is missing, and must be specified */
extern int const kPFErrorUserEmailMissing;
/** Error 205: A user with the specified email was not found */
extern int const kPFErrorUserWithEmailNotFound;
/** Error 206: The user cannot be altered by a client without the session. */
extern int const kPFErrorUserCannotBeAlteredWithoutSession;
/** Error 207: Users can only be created through sign up */
extern int const kPFErrorUserCanOnlyBeCreatedThroughSignUp;
/** Error 208: An existing Facebook account already linked to another user. */
extern int const kPFErrorFacebookAccountAlreadyLinked;
/** Error 208: An existing account already linked to another user. */
extern int const kPFErrorAccountAlreadyLinked;
/** Error 209: User ID mismatch */
extern int const kPFErrorUserIdMismatch;
/** Error 250: Facebook id missing from request */
extern int const kPFErrorFacebookIdMissing;
/** Error 250: Linked id missing from request */
extern int const kPFErrorLinkedIdMissing;
/** Error 251: Invalid Facebook session */
extern int const kPFErrorFacebookInvalidSession;
/** Error 251: Invalid linked session */
extern int const kPFErrorInvalidLinkedSession;

/** Error 300: File download failed due to connection issue */
extern int const kPFErrorFileDownloadConnectionFailed;

namespace parse {

class PFError : public QObject
{
public:

	// Creation Methods
	static PFErrorPtr errorWithCodeAndMessage(int code, const QString& message);

	// Error Accessor Methods
	int errorCode() const;
	const QString& errorMessage() const;

protected:

	// Constructor / Destructor
	PFError(int errorCode, const QString& errorMessage);
	~PFError();

	// Instance members
	int			_errorCode;
	QString		_errorMessage;
};

}	// End of parse namespace

QDebug operator<<(QDebug dbg, parse::PFErrorPtr error);

#endif	// End of PARSE_PFERROR_H
