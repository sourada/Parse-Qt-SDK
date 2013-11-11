//
//  PFUser.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include <Parse/PFManager.h>
#include <Parse/PFUser.h>

// Qt headers
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace parse {

// Static Globals
static QMutex		gMutex;
static PFUserPtr	gCurrentUser;

PFUser::PFUser() :
	_username(""),
	_email(""),
	_password(""),
	_objectId(""),
	_sessionToken(""),
	_createdAt(),
	_updatedAt()
{
	// No-op
}

PFUser::~PFUser()
{
	// No-op
}

PFUserPtr PFUser::currentUser()
{
	QMutexLocker locker(&gMutex);
	return gCurrentUser;
}

PFUserPtr PFUser::user()
{
	QMutexLocker locker(&gMutex);
	gCurrentUser = PFUserPtr(new PFUser());
	return gCurrentUser;
}

QVariant PFUser::variantWithUser(const PFUserPtr& user)
{
	QVariant variant;
	variant.setValue(user);
	return variant;
}

void PFUser::signUpInBackground(QObject* target, const char* action)
{
	QMutexLocker locker(&gMutex);

	// Create a network request
	QNetworkRequest request(QUrl("https://api.parse.com/1/users"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Create a JSON object out of our keys
	QJsonObject jsonObject;
	jsonObject["username"] = gCurrentUser->_username;
	jsonObject["email"] = gCurrentUser->_email;
	jsonObject["password"] = gCurrentUser->_password;
	QByteArray data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	networkAccessManager->post(request, data);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), PFManager::instance(), SLOT(handleSignUpReply(QNetworkReply*)));
	QObject::connect(PFManager::instance(), SIGNAL(signUpCompleted(bool, PFErrorPtr)), target, action);
}

PFUserPtr PFUser::logInWithUsernameAndPassword(const QString& username, const QString& password)
{
	QMutexLocker locker(&gMutex);

	// Make sure we're logged out
	if (!gCurrentUser.isNull())
		gCurrentUser = PFUserPtr();

	// Create the url string
	QUrlQuery urlQuery = QUrlQuery("https://api.parse.com/1/login?");
	urlQuery.addQueryItem("username", username);
	urlQuery.addQueryItem("password", password);
	QUrl url = QUrl(urlQuery.query());

	// Create a network request
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	QNetworkReply* reply = networkAccessManager->get(request);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	QJsonObject jsonObject = doc.object();
	qDebug() << "Log In Response:" << doc;

	// Notify the target of the success or failure
	if (reply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Create a new user
		gCurrentUser = PFUserPtr(new PFUser());
		gCurrentUser->_username = jsonObject["username"].toString();
		gCurrentUser->_email = jsonObject["email"].toString();
		gCurrentUser->_objectId = jsonObject["objectId"].toString();
		gCurrentUser->_sessionToken = jsonObject["sessionToken"].toString();
		QString createdAt = jsonObject["createdAt"].toString();
		gCurrentUser->_createdAt = PFDateTime::fromParseString(createdAt);
		QString updatedAt = jsonObject["updatedAt"].toString();
		gCurrentUser->_updatedAt = PFDateTime::fromParseString(updatedAt);
	}

	return gCurrentUser;
}

void PFUser::logInWithUsernameAndPasswordInBackground(const QString& username, const QString& password, QObject* target, const char* action)
{
	QMutexLocker locker(&gMutex);

	// Create the url string
	QUrlQuery urlQuery = QUrlQuery("https://api.parse.com/1/login?");
	urlQuery.addQueryItem("username", username);
	urlQuery.addQueryItem("password", password);
	QUrl url = QUrl(urlQuery.query());

	// Create a network request
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	networkAccessManager->get(request);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), PFManager::instance(), SLOT(handleLogInReply(QNetworkReply*)));
	QObject::connect(PFManager::instance(), SIGNAL(logInCompleted(bool, PFErrorPtr)), target, action);
}

void PFUser::logOut()
{
	QMutexLocker locker(&gMutex);

	// Reset the global user if allocated
	if (!gCurrentUser.isNull())
		gCurrentUser = PFUserPtr();
}

void PFUser::requestPasswordResetForEmailInBackground(const QString& email, QObject* target, const char* action)
{
	// Create a network request
	QNetworkRequest request(QUrl("https://api.parse.com/1/requestPasswordReset"));
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), QString("application/json").toUtf8());

	// Create a JSON object out of our keys
	QJsonObject jsonObject;
	jsonObject["email"] = email;
	QByteArray data = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	networkAccessManager->post(request, data);
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), PFManager::instance(), SLOT(handleRequestPasswordResetReply(QNetworkReply*)));
	QObject::connect(PFManager::instance(), SIGNAL(requestPasswordResetCompleted(bool, PFErrorPtr)), target, action);
}

bool PFUser::isAuthenticated()
{
	return !_sessionToken.isEmpty();
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

const QString& PFUser::objectId()
{
	return _objectId;
}

const QString& PFUser::sessionToken()
{
	return _sessionToken;
}

#pragma mark - Backend API - PFSerializable Methods

void PFUser::fromJson(const QJsonObject& jsonObject)
{
	qDebug() << "PFUser::fromJson";
	_objectId = jsonObject["objectId"].toString();
}

void PFUser::toJson(QJsonObject& jsonObject)
{
	qDebug() << "PFUser::toJson";
	if (_objectId.isEmpty())
		qFatal("PFFile::toJson could NOT convert to PFUser to JSON because the objectId is not set");
	jsonObject["__type"] = QString("Pointer");
	jsonObject["className"] = QString("_User");
	jsonObject["objectId"] = _objectId;
}

}	// End of parse namespace
