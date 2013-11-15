//
//  PFUser.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include "PFDateTime.h"
#include "PFError.h"
#include "PFManager.h"
#include "PFUser.h"

// Qt headers
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace parse {

// Static Globals
static PFUserPtr gCurrentUser;			// global - contains a valid user when that user has been authenticated
static PFUserPtr gSignUpUser;			// temp - used while signing in then discarded
static PFUserPtr gLogInUser;			// temp - used while logging in then discarded
static PFUserPtr gPasswordResetUser;    // temp - used while requesting a password reset then discarded

#pragma mark - Memory Management Methods

PFUser::PFUser() : PFObject(),
	_username(""),
	_email(""),
	_password(""),
	_sessionToken("")
{
	qDebug().nospace() << "Created PFUser(" << QString().sprintf("%8p", this) << ")";
	_parseClassName = "_User";
}

PFUser::~PFUser()
{
	qDebug().nospace() << "Destroyed PFUser(" << QString().sprintf("%8p", this) << ")";
}

#pragma mark - Static Creation Methods

PFUserPtr PFUser::currentUser()
{
	return gCurrentUser;
}

PFUserPtr PFUser::user()
{
	return PFUserPtr(new PFUser(), &QObject::deleteLater);
}

PFUserPtr PFUser::userWithObjectId(const QString& objectId)
{
	if (objectId.isEmpty())
	{
		qWarning() << "PFUser::userWithObjectId failed because the objectId is empty";
		return PFUserPtr();
	}
	else
	{
		PFUserPtr user = PFUser::user();
		user->_objectId = objectId;
		return user;
	}
}

PFUserPtr PFUser::userFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = PFSerializable::fromVariant(variant);
	if (!serializable.isNull())
		return serializable.objectCast<PFUser>();

	return PFUserPtr();
}

#pragma mark - Public Accessor Methods

bool PFUser::isAuthenticated()
{
	// The only way you can be authenticated as a user is if your pointer matches
	// that of the current user and you have a valid session token. If you do not
	// match the current user and have a session token, then the session token will
	// be cleared as it is in an invalid state.
	if (gCurrentUser.data() == this)
	{
		return !_sessionToken.isEmpty();
	}
	else
	{
		_sessionToken = QString("");
		return false;
	}
}

void PFUser::setUsername(const QString& username)
{
	_username = username;
}

void PFUser::setEmail(const QString& email)
{
	_email = email;
}

void PFUser::setPassword(const QString& password)
{
	_password = password;
}

const QString& PFUser::username()
{
	return _username;
}

const QString& PFUser::email()
{
	return _email;
}

const QString& PFUser::password()
{
	return _password;
}

const QString PFUser::parseClassName()
{
	return "_User";
}

const QString& PFUser::sessionToken()
{
	return _sessionToken;
}

#pragma mark - Sign In Methods

bool PFUser::signUpWithUser(PFUserPtr user)
{
	PFErrorPtr error;
	return PFUser::signUpWithUser(user, error);
}

bool PFUser::signUpWithUser(PFUserPtr user, PFErrorPtr& error)
{
	// Make sure we're logged out
	PFUser::logOut();

	// Create a network request and data
	QNetworkRequest request;
	QByteArray data;
	user->createSignUpNetworkRequest(request, data);

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->post(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply and return the result
	bool success = user->deserializeSignUpNetworkReply(networkReply, error);

	// Update our current user if we succeeded
	if (success)
		gCurrentUser = user;
	gSignUpUser = PFUserPtr();

	// Clean up
	networkReply->deleteLater();

	return success;
}

void PFUser::signUpWithUserInBackground(PFUserPtr user, QObject* target, const char* action)
{
	// Make sure we're logged out
	PFUser::logOut();

	// Keep the user around locally to ensure the callback succeeds
	gSignUpUser = user;

	// Create a network request and data
	QNetworkRequest request;
	QByteArray data;
	gSignUpUser->createSignUpNetworkRequest(request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->post(request, data);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), gSignUpUser.data(), SLOT(handleSignUpReply(QNetworkReply*)));
	QObject::connect(gSignUpUser.data(), SIGNAL(signUpCompleted(bool, PFErrorPtr)), target, action);
}

#pragma mark - Log In Methods

PFUserPtr PFUser::logInWithUsernameAndPassword(const QString& username, const QString& password)
{
	PFErrorPtr error;
	return PFUser::logInWithUsernameAndPassword(username, password, error);
}

PFUserPtr PFUser::logInWithUsernameAndPassword(const QString& username, const QString& password, PFErrorPtr& error)
{
	// Make sure we're logged out
	PFUser::logOut();

	// Create a new user to store our data in
	gLogInUser = PFUser::user();
	gLogInUser->_username = username;
	gLogInUser->_password = password;

	// Create the network request
	QNetworkRequest request = gLogInUser->createLogInNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(request);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = gLogInUser->deserializeLogInNetworkReply(networkReply, error);

	// Update our current user if we succeeded
	if (success)
		gCurrentUser = gLogInUser;
	gLogInUser = PFUserPtr();

	// Clean up
	networkReply->deleteLater();

	return gCurrentUser;
}

void PFUser::logInWithUsernameAndPasswordInBackground(const QString& username, const QString& password, QObject* target, const char* action)
{
	// Make sure we're logged out
	PFUser::logOut();

	// Create a new user to store our data in
	gLogInUser = PFUser::user();
	gLogInUser->_username = username;
	gLogInUser->_password = password;

	// Create the network request
	QNetworkRequest request = gLogInUser->createLogInNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->get(request);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), gLogInUser.data(), SLOT(handleLogInReply(QNetworkReply*)));
	QObject::connect(gLogInUser.data(), SIGNAL(logInCompleted(bool, PFErrorPtr)), target, action);
}

#pragma mark - Log Out Methods

void PFUser::logOut()
{
	// First clear the pointers session token (could have other shared pointers as well), then
	// clear the current user reference.
	if (!gCurrentUser.isNull())
		gCurrentUser->_sessionToken = QString("");
	gCurrentUser = PFUserPtr();
}

#pragma mark - Password Reset Methods

bool PFUser::requestPasswordResetForEmail(const QString& email)
{
	PFErrorPtr error;
	return PFUser::requestPasswordResetForEmail(email, error);
}

bool PFUser::requestPasswordResetForEmail(const QString& email, PFErrorPtr& error)
{
	// Create a new user to store our data in
	gPasswordResetUser = PFUser::user();
	gPasswordResetUser->_email = email;

	// Create a network request and data
	QNetworkRequest request;
	QByteArray data;
	gPasswordResetUser->createPasswordResetNetworkRequest(request, data);

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->post(request, data);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	bool success = gPasswordResetUser->deserializePasswordResetNetworkReply(networkReply, error);

	// Clean up
	networkReply->deleteLater();
	gPasswordResetUser = PFUserPtr();

	return success;
}

void PFUser::requestPasswordResetForEmailInBackground(const QString& email, QObject* target, const char* action)
{
	// Create a new user to store our data in
	gPasswordResetUser = PFUser::user();
	gPasswordResetUser->_email = email;

	// Create a network request and data
	QNetworkRequest request;
	QByteArray data;
	gPasswordResetUser->createPasswordResetNetworkRequest(request, data);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->post(request, data);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), gPasswordResetUser.data(), SLOT(handleRequestPasswordResetReply(QNetworkReply*)));
	QObject::connect(gPasswordResetUser.data(), SIGNAL(requestPasswordResetCompleted(bool, PFErrorPtr)), target, action);
}

#pragma mark - PFSerializable Methods

QVariant PFUser::fromJson(const QJsonObject& jsonObject)
{
	QString objectId = jsonObject["objectId"].toString();
	PFUserPtr user = PFUser::userWithObjectId(objectId);

	return PFSerializable::toVariant(user);
}

bool PFUser::toJson(QJsonObject& jsonObject)
{
	if (_objectId.isEmpty())
	{
		qWarning() << "PFFile::toJson could NOT convert to PFUser to JSON because the objectId is not set";
		return false;
	}
	else
	{
		jsonObject["__type"] = QString("Pointer");
		jsonObject["className"] = _parseClassName;
		jsonObject["objectId"] = _objectId;
		return true;
	}
}

const QString PFUser::className() const
{
	return "PFUser";
}

#pragma mark - Background Network Reply Completion Slots

void PFUser::handleSignUpReply(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeSignUpNetworkReply(networkReply, error);

	// Update our current user if we succeeded
	if (success)
		gCurrentUser = gSignUpUser;
	gSignUpUser = PFUserPtr();

	// Emit the signal that sign up has completed and then disconnect it
	emit signUpCompleted(success, error);
	this->disconnect(SIGNAL(signUpCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

void PFUser::handleLogInReply(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeLogInNetworkReply(networkReply, error);

	// Update our current user if we succeeded
	if (success)
		gCurrentUser = gLogInUser;
	gLogInUser = PFUserPtr();

	// Emit the signal that log in has completed and then disconnect it
	emit logInCompleted(success, error);
	this->disconnect(SIGNAL(logInCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

void PFUser::handleRequestPasswordResetReply(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializePasswordResetNetworkReply(networkReply, error);

	// Emit the signal that the password reset has completed and then disconnect it
	emit requestPasswordResetCompleted(success, error);
	this->disconnect(SIGNAL(requestPasswordResetCompleted(bool, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
	gPasswordResetUser = PFUserPtr();
}

#pragma mark - Protected Network Request Builder Methods

void PFUser::createSignUpNetworkRequest(QNetworkRequest& request, QByteArray& data)
{
	// Create a network request
	request = QNetworkRequest(QUrl("https://api.parse.com/1/users"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Create a JSON object out of all our properties
	QJsonObject jsonObject;
	foreach (const QString& key, _properties.keys())
	{
		QVariant objectToSerialize = _properties[key];
		jsonObject[key] = convertDataToJson(objectToSerialize);
	}

	// Add the keys not tracked in the child objects
	jsonObject["username"] = _username;
	jsonObject["email"] = _email;
	jsonObject["password"] = _password;

	// Convert the json object into compact JSON
	data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}

QNetworkRequest PFUser::createLogInNetworkRequest()
{
	// Create the url string
	QUrlQuery urlQuery = QUrlQuery("https://api.parse.com/1/login?");
	urlQuery.addQueryItem("username", gLogInUser->_username);
	urlQuery.addQueryItem("password", gLogInUser->_password);
	QUrl url = QUrl(urlQuery.query());

	// Create a network request
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	return request;
}

void PFUser::createPasswordResetNetworkRequest(QNetworkRequest& request, QByteArray& data)
{
	// Create a network request
	request = QNetworkRequest(QUrl("https://api.parse.com/1/requestPasswordReset"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Create a JSON object out of our keys
	QJsonObject jsonObject;
	jsonObject["email"] = _email;
	data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}

#pragma mark - Network Request Builder Methods - PFObject Overrides

void PFUser::createSaveNetworkRequest(QNetworkRequest& request, QByteArray& data)
{
	if (!isAuthenticated())
	{
		qFatal("PFUser::createSaveNetworkRequest failed because the user has not been authenticated");
	}
	else
	{
		// Create the request to use for saving the user
		QUrl url = QUrl(QString("https://api.parse.com/1/users/") + _objectId);
		request = QNetworkRequest(url);
		request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
		request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), _sessionToken.toUtf8());
		request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

		// Call the parent implementation to build the data (use a dummy object for the request)
		QNetworkRequest dummyRequest;
		PFObject::createSaveNetworkRequest(dummyRequest, data);
	}
}

QNetworkRequest PFUser::createDeleteObjectNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/users/") + _objectId);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

QNetworkRequest PFUser::createFetchNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/users/") + _objectId);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

#pragma mark - Protected Network Reply Deserialization Methods

bool PFUser::deserializeSignUpNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		_objectId = jsonObject["objectId"].toString();
		_sessionToken = jsonObject["sessionToken"].toString();
		QString createdAt = jsonObject["createdAt"].toString();
		_createdAt = PFDateTime::dateTimeFromParseString(createdAt);

		return true;
	}
	else // FAILURE
	{
		// Create the error
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

bool PFUser::deserializeLogInNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Deserialize the json into our properties variant map and strip out the instance members
		_properties = convertJsonToVariant(jsonObject).toMap();
		stripInstanceMembersFromProperties();

		return true;
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

bool PFUser::deserializePasswordResetNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		return true;
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}

	return false;
}

bool PFUser::deserializeFetchNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Use the default PFObject deserialization for the fetch reply
	bool success = PFObject::deserializeFetchNetworkReply(networkReply, error);

	// If it succeeded, let's extract the username and email properties
	if (success)
		stripInstanceMembersFromProperties();

	return success;
}

#pragma mark - Recursive JSON Conversion Helper Methods
void PFUser::stripInstanceMembersFromProperties()
{
	// First make sure to call the parent method to get our base stripping to occur
	PFObject::stripInstanceMembersFromProperties();

	// Now let's strip out the PFUser specific properties

	// username
	if (_properties.contains("username"))
		_username = _properties.take("username").toString();

	// email
	if (_properties.contains("email"))
		_email = _properties.take("email").toString();

	// sessionToken
	if (_properties.contains("sessionToken"))
		_sessionToken = _properties.take("sessionToken").toString();
}

}	// End of parse namespace
