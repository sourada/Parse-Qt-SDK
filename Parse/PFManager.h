//
//  PFManager.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFMANAGER_H
#define PARSE_PFMANAGER_H

// Parse headers
#include <Parse/PFError.h>

// Qt headers
#include <QDir>
#include <QNetworkAccessManager>
#include <QString>

namespace parse {

class PFManager : public QObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	/** Creates a singleton instance of the PFManager. */
	static PFManager* instance();

	/** Sets the application id and rest api key. */
	void setApplicationIdAndRestApiKey(const QString& applicationId, const QString& restApiKey);

	/** Accessor Methods. */
	const QString& applicationId();
	const QString& restApiKey();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	QNetworkAccessManager* networkAccessManager();
	QDir& cacheDirectory();
	void setCacheDirectory(const QDir& cacheDirectory);

protected slots:

	// PFUser completion slots
	void handleSignUpReply(QNetworkReply* networkReply);
	void handleLogInReply(QNetworkReply* networkReply);
	void handleRequestPasswordResetReply(QNetworkReply* networkReply);

signals:

	// PFUser completion signals
	void signUpCompleted(bool succeeded, PFErrorPtr error);
	void logInCompleted(bool succeeded, PFErrorPtr error);
	void requestPasswordResetCompleted(bool succeeded, PFErrorPtr error);

protected:

	PFManager();
	~PFManager();

	void registerMetaTypesForSignalSlots();

	/** Instance members. */
	QString					_applicationId;
	QString					_restApiKey;
	QDir					_cacheDirectory;
	QNetworkAccessManager	_networkAccessManager;
};

}	// End of parse namespace

#endif	// End of PARSE_PFMANAGER_H
