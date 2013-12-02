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

	// Sorting Methods
	void orderByAscending(const QString& key);
	void orderByDescending(const QString& key);
	void addAscendingOrder(const QString& key);
	void addDescendingOrder(const QString& key);

	// Get Object Methods - getObjectCompleteAction signature: (PFObjectPtr object, PFErrorPtr error)
	static PFObjectPtr getObjectOfClassWithId(const QString& className, const QString& objectId);
	static PFObjectPtr getObjectOfClassWithId(const QString& className, const QString& objectId, PFErrorPtr& error);
	PFObjectPtr getObjectWithId(const QString& objectId);
	PFObjectPtr getObjectWithId(const QString& objectId, PFErrorPtr& error);
	void getObjectWithIdInBackground(const QString& objectId, QObject* getObjectCompleteTarget, const char* getObjectCompleteAction);

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
	void handleGetObjectCompleted(QNetworkReply* networkReply);
	void handleFindObjectsCompleted(QNetworkReply* networkReply);

signals:

	// Background Request Completion Signals
	void getObjectCompleted(PFObjectPtr object, PFErrorPtr error);
	void findObjectsCompleted(PFObjectList objects, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFQuery();
	~PFQuery();

	// Network Request Builder Methods
	QNetworkRequest createGetObjectNetworkRequest();
	QNetworkRequest createFindObjectsNetworkRequest();

	// Network Reply Deserialization Methods
	PFObjectPtr deserializeGetObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	PFObjectList deserializeFindObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Key Helper Methods
	void addWhereOption(const QString& key, const QString& option, const QVariant& object);
	QJsonObject fetchWhereKeyObject(const QString& key);

	// Instance members
	QString			_className;
	QVariantMap		_whereMap;
	QSet<QString>	_whereEqualKeys;
	QStringList		_orderKeys;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFQueryPtr)

#endif	// End of PARSE_PFQUERY_H
