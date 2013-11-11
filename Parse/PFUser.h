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
#include <Parse/PFDateTime.h>
#include <Parse/PFError.h>
#include <Parse/PFSerializable.h>

// Qt headers
#include <QDateTime>
#include <QNetworkReply>
#include <QObject>
#include <QSharedPointer>
#include <QString>

namespace parse {

// Typedefs
class PFUser;
typedef QSharedPointer<PFUser> PFUserPtr;

class PFUser : public PFSerializable
{
public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	static PFUserPtr currentUser();
	static PFUserPtr user();

	/** Helper method for creating a QVariant with a PFUserPtr object. */
	static QVariant variantWithUser(const PFUserPtr& user);

	static void signUpInBackground(QObject* target, const char* action);

	static PFUserPtr logInWithUsernameAndPassword(const QString& username, const QString& password);
	static void logInWithUsernameAndPasswordInBackground(const QString& username, const QString& password, QObject* target, const char* action);
	static void logOut();

	static void requestPasswordResetForEmailInBackground(const QString& email, QObject* target, const char* action);

	bool isAuthenticated();

	void setUsername(const QString& username);
	void setEmail(const QString& email);
	void setPassword(const QString& password);

	const QString& username();
	const QString& email();
	const QString& password();
	const QString& objectId();
	const QString& sessionToken();

	/** Destructor. */
	virtual ~PFUser();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	void fromJson(const QJsonObject& jsonObject);
	void toJson(QJsonObject& jsonObject);

	/** Declare the PFManager to be a friend class. */
	friend class PFManager;

protected:

	/** Constructor. */
	PFUser();

	/** Instance members. */
	QString						_username;
	QString						_email;
	QString						_password;
	QString						_objectId;
	QString						_sessionToken;
	PFDateTime					_createdAt;
	PFDateTime					_updatedAt;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFUserPtr)

#endif	// End of PARSE_PFUSER_H
