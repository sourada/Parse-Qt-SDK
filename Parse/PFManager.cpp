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

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFManager::PFManager() :
	_applicationId(""),
	_restApiKey(""),
	_masterKey(""),
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

#ifdef __APPLE__
#pragma mark - Creation Methods
#endif

PFManager* PFManager::sharedManager()
{
	QMutexLocker lock(&gPFManagerMutex);
	static PFManager manager;
	return &manager;
}

#ifdef __APPLE__
#pragma mark - User API
#endif

void PFManager::setApplicationIdAndRestApiKey(const QString& applicationId, const QString& restApiKey)
{
	_applicationId = applicationId;
	_restApiKey = restApiKey;
}

void PFManager::setMasterKey(const QString& masterKey)
{
	_masterKey = masterKey;
}

const QString& PFManager::applicationId()
{
	return _applicationId;
}

const QString& PFManager::restApiKey()
{
	return _restApiKey;
}

const QString& PFManager::masterKey()
{
	return _masterKey;
}

#ifdef __APPLE__
#pragma mark - Backend API - Caching and Network Methods
#endif

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
