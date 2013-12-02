//
//  PFQuery.h
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFQUERY_H
#define PARSE_PFQUERY_H

// Parse headers
#include "PFTypedefs.h"

// Qt headers
#include <QNetworkReply>
#include <QNetworkRequest>

namespace parse {

class PFQuery : public QObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creation Methods
	static PFQueryPtr queryWithClassName(const QString& className);

	// Key Constraint Methods
	void whereKeyEqualTo(const QString& key, const QVariant& object);
	void whereKeyNotEqualTo(const QString& key, const QVariant& object);

	// Find Objects Methods - findCompleteAction signature: (bool succeeded, PFErrorPtr error)
	PFObjectList findObjects();
	PFObjectList findObjects(PFErrorPtr& error);
	void findObjectsInBackground(QObject* findCompleteTarget, const char* findCompleteAction);

	// Accessor Methods
	const QString& className();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

protected slots:

	// Background Network Reply Completion Slots
	void handleFindObjectsCompleted(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void findObjectsCompleted(PFObjectList objects, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFQuery();
	~PFQuery();

	// Network Request Builder Methods
	QNetworkRequest createFindObjectsNetworkRequest();

	// Network Reply Deserialization Methods
	PFObjectList deserializeFindObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Key Helper Methods
	void addWhereOption(const QString& key, const QString& option, const QVariant& object);
	QJsonObject fetchWhereKeyObject(const QString& key);

	// Instance members
	QString			_className;
	QVariantMap		_whereMap;
	QSet<QString>	_whereEqualKeys;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFQueryPtr)

#endif	// End of PARSE_PFQUERY_H
