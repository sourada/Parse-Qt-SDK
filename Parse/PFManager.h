//
//  PFManager.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFMANAGER_H
#define PARSE_PFMANAGER_H

// Qt headers
#include <QDir>
#include <QNetworkAccessManager>
#include <QString>

namespace parse {

class PFManager : public QObject
{
public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creates a singleton instance of the PFManager
	static PFManager* sharedManager();

	// Sets the application id and rest api key
	void setApplicationIdAndRestApiKey(const QString& applicationId, const QString& restApiKey);

	// Application ID, Rest API Key Getter Methods
	const QString& applicationId();
	const QString& restApiKey();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// Caching and Network Methods
	QNetworkAccessManager* networkAccessManager();
	void setCacheDirectory(const QDir& cacheDirectory);
	QDir& cacheDirectory();
	void clearCache();

protected:

	// Constructor / Destructor
	PFManager();
	~PFManager();

	// Instance members
	QString					_applicationId;
	QString					_restApiKey;
	QDir					_cacheDirectory;
	QNetworkAccessManager	_networkAccessManager;
};

}	// End of parse namespace

#endif	// End of PARSE_PFMANAGER_H
