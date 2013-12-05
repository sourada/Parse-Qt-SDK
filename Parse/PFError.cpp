//
//  PFError.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFError.h"

// Qt headers
#include <QDebug>

extern int const kPFErrorInternalServer = 1;

extern int const kPFErrorConnectionFailed = 100;
extern int const kPFErrorObjectNotFound = 101;
extern int const kPFErrorInvalidQuery = 102;
extern int const kPFErrorInvalidClassName = 103;
extern int const kPFErrorMissingObjectId = 104;
extern int const kPFErrorInvalidKeyName = 105;
extern int const kPFErrorInvalidPointer = 106;
extern int const kPFErrorInvalidJSON = 107;
extern int const kPFErrorCommandUnavailable = 108;
extern int const kPFErrorIncorrectType = 111;
extern int const kPFErrorInvalidChannelName = 112;
extern int const kPFErrorInvalidDeviceToken = 114;
extern int const kPFErrorPushMisconfigured = 115;
extern int const kPFErrorObjectTooLarge = 116;
extern int const kPFErrorOperationForbidden = 119;
extern int const kPFErrorCacheMiss = 120;
extern int const kPFErrorInvalidNestedKey = 121;
extern int const kPFErrorInvalidFileName = 122;
extern int const kPFErrorInvalidACL = 123;
extern int const kPFErrorTimeout = 124;
extern int const kPFErrorInvalidEmailAddress = 125;
extern int const kPFErrorDuplicateValue = 137;
extern int const kPFErrorInvalidRoleName = 139;
extern int const kPFErrorExceededQuota = 140;
extern int const kPFScriptError = 141;
extern int const kPFValidationError = 142;
extern int const kPFErrorReceiptMissing = 143;
extern int const kPFErrorInvalidPurchaseReceipt = 144;
extern int const kPFErrorPaymentDisabled = 145;
extern int const kPFErrorInvalidProductIdentifier = 146;
extern int const kPFErrorProductNotFoundInAppStore = 147;
extern int const kPFErrorInvalidServerResponse = 148;
extern int const kPFErrorProductDownloadFileSystemFailure = 149;
extern int const kPFErrorInvalidImageData = 150;
extern int const kPFErrorUnsavedFile = 151;
extern int const kPFErrorFileDeleteFailure = 152;

extern int const kPFErrorUsernameMissing = 200;
extern int const kPFErrorUserPasswordMissing = 201;
extern int const kPFErrorUsernameTaken = 202;
extern int const kPFErrorUserEmailTaken = 203;
extern int const kPFErrorUserEmailMissing = 204;
extern int const kPFErrorUserWithEmailNotFound = 205;
extern int const kPFErrorUserCannotBeAlteredWithoutSession = 206;
extern int const kPFErrorUserCanOnlyBeCreatedThroughSignUp = 207;
extern int const kPFErrorFacebookAccountAlreadyLinked = 208;
extern int const kPFErrorAccountAlreadyLinked = 208;
extern int const kPFErrorUserIdMismatch = 209;
extern int const kPFErrorFacebookIdMissing = 250;
extern int const kPFErrorLinkedIdMissing = 250;
extern int const kPFErrorFacebookInvalidSession = 251;
extern int const kPFErrorInvalidLinkedSession = 251;

extern int const kPFErrorFileDownloadConnectionFailed = 300;

namespace parse {

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFError::PFError(int errorCode, const QString& errorMessage) :
	_errorCode(errorCode),
	_errorMessage(errorMessage)
{
	qDebug().nospace() << "Created PFError(" << QString().sprintf("%8p", this) << ")";
}

PFError::~PFError()
{
	qDebug().nospace() << "Destroyed PFError(" << QString().sprintf("%8p", this) << ")";
}

#ifdef __APPLE__
#pragma mark - Static Creation Methods
#endif

PFErrorPtr PFError::errorWithCodeAndMessage(int code, const QString& message)
{
	PFErrorPtr error = PFErrorPtr(new PFError(code, message), &QObject::deleteLater);
	return error;
}

#ifdef __APPLE__
#pragma mark - Error Access Methods
#endif

int PFError::errorCode() const
{
	return _errorCode;
}

const QString& PFError::errorMessage() const
{
	return _errorMessage;
}

}	// End of parse namespace

#ifdef __APPLE__
#pragma mark - Custom Debug Output
#endif

QDebug operator<<(QDebug dbg, parse::PFErrorPtr error)
{
	QString message = "NULL";
	if (!error.isNull())
		message = QString::number(error->errorCode()) + " " + error->errorMessage();
	dbg.nospace() << "PFError(" << message << ")";
	return dbg.maybeSpace();
}
