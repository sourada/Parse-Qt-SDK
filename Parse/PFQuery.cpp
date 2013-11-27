//
//  PFQuery.cpp
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFError.h"
#include "PFManager.h"
#include "PFObject.h"
#include "PFQuery.h"
#include "PFUser.h"

// Qt headers
#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>

namespace parse {

#pragma mark - Memory Management Methods

PFQuery::PFQuery()
{
	qDebug().nospace() << "Created PFQuery(" << QString().sprintf("%8p", this) << ")";

	// Set up ivar defaults
	_className = "";
}

PFQuery::~PFQuery()
{
	qDebug().nospace() << "Destroyed PFQuery(" << QString().sprintf("%8p", this) << ")";
}

#pragma mark - Creation Methods

PFQueryPtr PFQuery::queryWithClassName(const QString& className)
{
	if (className.isEmpty())
	{
		qWarning() << "PFQuery::queryWithClassName failed to create a new PFQuery because the className was empty";
		return PFQueryPtr();
	}
	else
	{
		PFQueryPtr query = PFQueryPtr(new PFQuery(), &QObject::deleteLater);
		query->_className = className;
		return query;
	}
}

#pragma mark - Find Objects Methods

PFObjectList PFQuery::findObjects()
{
	PFErrorPtr error;
	return findObjects(error);
}

PFObjectList PFQuery::findObjects(PFErrorPtr& error)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createFindObjectsNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->get(networkRequest);

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Deserialize the reply
	const PFObjectList& objects = deserializeFindObjectsNetworkReply(networkReply, error);

	return objects;
}

void PFQuery::findObjectsInBackground(QObject* findCompleteTarget, const char* findCompleteAction)
{
	// Prep the request and data
	QNetworkRequest networkRequest = createFindObjectsNetworkRequest();

	// Execute the request
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->get(networkRequest);

	// Connect all the callbacks
	QObject::connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleFindObjectsCompleted(QNetworkReply*)));
	QObject::connect(this, SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)), findCompleteTarget, findCompleteAction);
}

#pragma mark - Accessor Methods

const QString& PFQuery::className()
{
	return _className;
}

#pragma mark - Background Network Reply Completion Slots

void PFQuery::handleFindObjectsCompleted(QNetworkReply* networkReply)
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);

	// Deserialize the reply
	PFErrorPtr error;
	const PFObjectList& objects = deserializeFindObjectsNetworkReply(networkReply, error);

	// Emit the signal that the request completed and then disconnect it
	emit findObjectsCompleted(objects, error);
	this->disconnect(SIGNAL(findObjectsCompleted(PFObjectList, PFErrorPtr)));

	// Clean up
	networkReply->deleteLater();
}

#pragma mark - Network Request Builder Methods

QNetworkRequest PFQuery::createFindObjectsNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/classes/") + _className);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());

	// Attach the session token if we're authenticated as a particular user
	if (PFUser::currentUser() && PFUser::currentUser()->isAuthenticated())
		request.setRawHeader(QString("X-Parse-Session-Token").toUtf8(), PFUser::currentUser()->sessionToken().toUtf8());

	return request;
}

#pragma mark - Network Reply Deserialization Methods

PFObjectList PFQuery::deserializeFindObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Create the list of objects to return
	PFObjectList objects;

	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Extract the results array
		QJsonObject rootObject = doc.object();
		QJsonArray results = rootObject["results"].toArray();

		// Go through each item in the reply and create a PFObject out of it
		foreach (const QJsonValue& resultValue, results)
		{
			// Grab the result json object and add the className property to it
			QJsonObject resultObject = resultValue.toObject();
			resultObject["className"] = _className;

			// Convert the json to a PFObject and store it
			QVariant objectVariant = PFObject::fromJson(resultObject);
			PFObjectPtr object = PFObject::objectFromVariant(objectVariant);
			objects.append(object);
		}
	}
	else // FAILURE
	{
		QJsonObject jsonObject = doc.object();
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);
	}

	return objects;
}

}	// End of parse namespace