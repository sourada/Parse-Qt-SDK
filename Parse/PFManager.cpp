//
//  PFManager.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Qt headers
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkReply>

// Parse headers
#include <Parse/PFDateTime.h>
#include <Parse/PFManager.h>
#include <Parse/PFUser.h>

namespace parse {

// Static Globals
static QMutex	gPFManagerMutex;

#pragma mark - Memory Management Methods

PFManager::PFManager() :
	_applicationId(""),
	_restApiKey(""),
	_cacheDirectory(""),
	_networkAccessManager()
{
	// Define the default cache directory as $$TMPDIR/Parse
	_cacheDirectory = QDir::temp();
	_cacheDirectory.mkdir("Parse");
	_cacheDirectory.cd("Parse");

	// Register the Parse metatypes
	registerMetaTypesForSignalSlots();
}

PFManager::~PFManager()
{
	// No-op
}

PFManager* PFManager::instance()
{
	QMutexLocker lock(&gPFManagerMutex);
	static PFManager manager;
	return &manager;
}

#pragma mark - User API

void PFManager::setApplicationIdAndRestApiKey(const QString& applicationId, const QString& restApiKey)
{
	_applicationId = applicationId;
	_restApiKey = restApiKey;
}

const QString& PFManager::applicationId()
{
	return _applicationId;
}

const QString& PFManager::restApiKey()
{
	return _restApiKey;
}

#pragma mark - Backend API - Public Methods

QNetworkAccessManager* PFManager::networkAccessManager()
{
	return &_networkAccessManager;
}

QDir& PFManager::cacheDirectory()
{
	return _cacheDirectory;
}

void PFManager::setCacheDirectory(const QDir& cacheDirectory)
{
	_cacheDirectory = cacheDirectory;
}

#pragma mark - Backend API - PFUser Network Replies

void PFManager::handleSignUpReply(QNetworkReply* networkReply)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Notify the target of the success or failure
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		PFUserPtr currentUser = PFUser::currentUser();
		currentUser->_objectId = jsonObject["objectId"].toString();
		currentUser->_sessionToken = jsonObject["sessionToken"].toString();
		QString createdAt = jsonObject["createdAt"].toString();
		currentUser->_createdAt = PFDateTimePtr(new PFDateTime(createdAt));
		emit signUpCompleted(true, PFErrorPtr());
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		emit signUpCompleted(false, PFErrorPtr(new PFError(errorCode, errorMessage)));
	}

	// Clean up
	networkReply->deleteLater();
}

void PFManager::handleLogInReply(QNetworkReply* networkReply)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Notify the target of the success or failure
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// If we're already logged in, then log out and then create a new user
		if (!PFUser::currentUser().isNull())
			PFUser::logOut();

		// Create a new user
		PFUserPtr currentUser = PFUser::user();
		currentUser->_username = jsonObject["username"].toString();
		currentUser->_email = jsonObject["email"].toString();
		currentUser->_objectId = jsonObject["objectId"].toString();
		currentUser->_sessionToken = jsonObject["sessionToken"].toString();
		QString createdAt = jsonObject["createdAt"].toString();
		currentUser->_createdAt = PFDateTimePtr(new PFDateTime(createdAt));
		QString updatedAt = jsonObject["updatedAt"].toString();
		currentUser->_updatedAt = PFDateTimePtr(new PFDateTime(updatedAt));
		emit logInCompleted(true, PFErrorPtr());
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		emit logInCompleted(false, PFErrorPtr(new PFError(errorCode, errorMessage)));
	}

	// Clean up
	networkReply->deleteLater();
}

void PFManager::handleRequestPasswordResetReply(QNetworkReply* networkReply)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Notify the target of the success or failure
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Successful reply has no payload
		emit requestPasswordResetCompleted(true, PFErrorPtr());
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		emit requestPasswordResetCompleted(false, PFErrorPtr(new PFError(errorCode, errorMessage)));
	}

	// Clean up
	networkReply->deleteLater();
}

#pragma mark - Protected Methods

void PFManager::registerMetaTypesForSignalSlots()
{
	// Register the typedefs for signals and slots
	qRegisterMetaType<parse::PFErrorPtr>("PFErrorPtr");
	qRegisterMetaType<parse::PFUserPtr>("PFUserPtr");
}

}	// End of parse namespace
