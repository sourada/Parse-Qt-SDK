//
//  PFManager.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFManager.h"

// Qt headers
#include <QMutex>
#include <QMutexLocker>

namespace parse {

// Static Globals
static QMutex gPFManagerMutex;

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
	qDebug() << "Cache directory:" << _cacheDirectory.absolutePath();
}

PFManager::~PFManager()
{
	// No-op
}

#pragma mark - Creation Methods

PFManager* PFManager::sharedManager()
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

#pragma mark - Backend API - Caching and Network Methods

QNetworkAccessManager* PFManager::networkAccessManager()
{
	return &_networkAccessManager;
}

void PFManager::setCacheDirectory(const QDir& cacheDirectory)
{
	_cacheDirectory = cacheDirectory;
}

QDir& PFManager::cacheDirectory()
{
	return _cacheDirectory;
}

void PFManager::clearCache()
{
	_cacheDirectory.removeRecursively();
	_cacheDirectory.mkpath(_cacheDirectory.absolutePath());
}

}	// End of parse namespace
