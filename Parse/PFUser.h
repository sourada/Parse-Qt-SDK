//
//  PFUser.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFUSER_H
#define PARSE_PFUSER_H

// Parse headers
#include "PFObject.h"

// Qt headers
#include <QNetworkReply>

namespace parse {

class PFUser : public PFObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creation Methods
	static PFUserPtr user();
	static PFUserPtr userWithObjectId(const QString& objectId);
	static PFUserPtr userFromVariant(const QVariant& variant);

	// Returns the currently authenticated user (NULL if the user isn't logged in)
	static PFUserPtr currentUser(); // Only returns a valid user when authenticated

	// Returns whether the user has been authenticated
	bool isAuthenticated();

	// Username, Email, Password Accessors
	void setUsername(const QString& username);
	void setEmail(const QString& email);
	void setPassword(const QString& password);
	const QString& username();
	const QString& email();
	const QString& password();

	// Object ID, Session Token Getter Methods
	virtual const QString parseClassName();
	const QString& sessionToken();

	////////////////////////////////
	//       Sign Up Methods
	////////////////////////////////

	static bool signUpWithUser(PFUserPtr user);
	static bool signUpWithUser(PFUserPtr user, PFErrorPtr& error);

	// Attempts to sign up asynchronously and notifies the target / action pair upon completion
	//   @param user The user object to sign up with Parse
	//   @param target The target to be notified when the sign up completes
	//   @param action The slot to be notified when the sign up completes - SLOT(signUpCompleted(bool, PFErrorPtr))
	static void signUpWithUserInBackground(PFUserPtr user, QObject* target, const char* action);

	////////////////////////////////
	//       Log In Methods
	////////////////////////////////

	static PFUserPtr logInWithUsernameAndPassword(const QString& username, const QString& password);
	static PFUserPtr logInWithUsernameAndPassword(const QString& username, const QString& password, PFErrorPtr& error);

	// Attempts to log in asynchronously and notifies the target / action pair upon completion
	//   @param username The username to use when logging in to Parse
	//   @param password The password to use when logging in to Parse
	//   @param target The target to be notified when the log in completes
	//   @param action The slot to be notified when the log in completes - SLOT(logInCompleted(bool, PFErrorPtr))
	static void logInWithUsernameAndPasswordInBackground(const QString& username, const QString& password, QObject* target, const char* action);

	////////////////////////////////
	//       Log Out Methods
	////////////////////////////////

	static void logOut();

	////////////////////////////////
	//   Password Reset Methods
	////////////////////////////////

	static bool requestPasswordResetForEmail(const QString& email);
	static bool requestPasswordResetForEmail(const QString& email, PFErrorPtr& error);

	// Attempts to request a password reset for the given email and notifies the target / action pair upon completion
	//   @param email The email to try to reset the password for
	//   @param target The target to be notified when the password reset request completes
	//   @param action The slot to be notified when the password reset request completes - SLOT(requestPasswordResetCompleted(bool, PFErrorPtr))
	static void requestPasswordResetForEmailInBackground(const QString& email, QObject* target, const char* action);

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	static QVariant fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString className() const;

protected slots:

	// Background Network Reply Completion Slots
	void handleSignUpReply(QNetworkReply* networkReply);
	void handleLogInReply(QNetworkReply* networkReply);
	void handleRequestPasswordResetReply(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void signUpCompleted(bool succeeded, PFErrorPtr error);
	void logInCompleted(bool succeeded, PFErrorPtr error);
	void requestPasswordResetCompleted(bool succeeded, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFUser();
	virtual ~PFUser();

	// Network Request Builder Methods
	void createSignUpNetworkRequest(QNetworkRequest& request, QByteArray& data);
	QNetworkRequest createLogInNetworkRequest();
	void createPasswordResetNetworkRequest(QNetworkRequest& request, QByteArray& data);

	// Network Request Builder Methods - PFObject Overrides
	virtual void createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data);
	virtual QNetworkRequest createDeleteObjectNetworkRequest();
	virtual QNetworkRequest createFetchNetworkRequest();

	// Network Reply Deserialization Methods
	bool deserializeSignUpNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	bool deserializeLogInNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	bool deserializePasswordResetNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	virtual bool deserializeFetchNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Recursive JSON Conversion Helper Methods
	virtual void stripInstanceMembersFromProperties();

	// Instance members
	QString			_username;
	QString			_email;
	QString			_password;
	QString			_sessionToken;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFUserPtr)

#endif	// End of PARSE_PFUSER_H
