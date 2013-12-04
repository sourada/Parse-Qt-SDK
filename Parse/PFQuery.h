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

	////////////////////////////////
	//      Creation Methods
	////////////////////////////////

	static PFQueryPtr queryWithClassName(const QString& className);

	////////////////////////////////
	//        Query Options
	////////////////////////////////

	// Key Inclusion/Exclusion
	void includeKey(const QString& key);
	void selectKeys(const QStringList& keys);

	// Key Constraints - Object Comparisons
	void whereKeyExists(const QString& key);
	void whereKeyDoesNotExist(const QString& key);
	void whereKeyEqualTo(const QString& key, const QVariant& object);
	void whereKeyNotEqualTo(const QString& key, const QVariant& object);
	void whereKeyLessThan(const QString& key, const QVariant& object);
	void whereKeyLessThanOrEqualTo(const QString& key, const QVariant& object);
	void whereKeyGreaterThan(const QString& key, const QVariant& object);
	void whereKeyGreaterThanOrEqualTo(const QString& key, const QVariant& object);

	// Key Constraints - List Comparisons
	void whereKeyContainedIn(const QString& key, const QVariantList& objects);
	void whereKeyNotContainedIn(const QString& key, const QVariantList& objects);
	void whereKeyContainsAllObjects(const QString& key, const QVariantList& objects);

	////////////////////////////////
	//       Sorting Methods
	////////////////////////////////

	// Sort the results in ascending or descending order
	void orderByAscending(const QString& key);
	void orderByDescending(const QString& key);
	void addAscendingOrder(const QString& key);
	void addDescendingOrder(const QString& key);

	////////////////////////////////
	//      Pagination Methods
	////////////////////////////////

	// A limit on the number of objects to return. The default limit is 100, with a
	// maximum of 1000 results being returned at a time.
	void setLimit(int limit);
	int limit();

	// The number of objects to skip before returning any
	void setSkip(int skip);
	int skip();

	////////////////////////////////
	//     Get Object Methods
	////////////////////////////////

	// Convenience methods for getting a single object
	static PFObjectPtr getObjectOfClassWithId(const QString& className, const QString& objectId);
	static PFObjectPtr getObjectOfClassWithId(const QString& className, const QString& objectId, PFErrorPtr& error);

	// Return a PFObject with the given object id - action signature: (PFObjectPtr object, PFErrorPtr error)
	PFObjectPtr getObjectWithId(const QString& objectId);
	PFObjectPtr getObjectWithId(const QString& objectId, PFErrorPtr& error);
	void getObjectWithIdInBackground(const QString& objectId, QObject* target, const char* action);

	////////////////////////////////
	//      Get User Methods
	////////////////////////////////

	// Returns a PFUser with the given object id
	static PFUserPtr getUserWithId(const QString& objectId);
	static PFUserPtr getUserWithId(const QString& objectId, PFErrorPtr& error);

	////////////////////////////////
	//     Find Objects Methods
	////////////////////////////////

	// Finds objects based on the constructed query - action signature: (bool succeeded, PFErrorPtr error)
	PFObjectList findObjects();
	PFObjectList findObjects(PFErrorPtr& error);
	void findObjectsInBackground(QObject* target, const char* action);

	////////////////////////////////
	//   Get First Object Methods
	////////////////////////////////

	// Gets an object based on the constructed query - action signature: (PFObjectPtr object, PFErrorPtr error)
	PFObjectPtr getFirstObject();
	PFObjectPtr getFirstObject(PFErrorPtr& error);
	void getFirstObjectInBackground(QObject* target, const char* action);

	////////////////////////////////
	//    Count Objects Methods
	////////////////////////////////

	// Counts objects based on the constructed query - action signature: (int count, PFErrorPtr error)
	int countObjects();
	int countObjects(PFErrorPtr& error);
	void countObjectsInBackground(QObject* target, const char* action);

	////////////////////////////////
	//    Cancellation Methods
	////////////////////////////////

	// Cancels the current network request (if any). Ensures callbacks won't be called.
	void cancel();

	////////////////////////////////
	//        Accessors
	////////////////////////////////

	const QString& className();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

protected slots:

	// Background Network Reply Completion Slots
	void handleGetObjectCompleted();
	void handleFindObjectsCompleted();
	void handleGetFirstObjectCompleted();
	void handleCountObjectsCompleted();

signals:

	// Background Request Completion Signals
	void getObjectCompleted(PFObjectPtr object, PFErrorPtr error);
	void findObjectsCompleted(PFObjectList objects, PFErrorPtr error);
	void getFirstObjectCompleted(PFObjectPtr object, PFErrorPtr error);
	void countObjectsCompleted(int count, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFQuery();
	~PFQuery();

	// Network Request Builder Methods
	QNetworkRequest createGetObjectNetworkRequest();
	QNetworkRequest createGetUserNetworkRequest();
	QNetworkRequest createFindObjectsNetworkRequest();
	QNetworkRequest createGetFirstObjectNetworkRequest();
	QNetworkRequest createCountObjectsNetworkRequest();

	// Network Reply Deserialization Methods
	PFObjectPtr deserializeGetObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	PFUserPtr deserializeGetUserNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	PFObjectList deserializeFindObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	PFObjectPtr deserializeGetFirstObjectNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	int deserializeCountObjectsNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Protected Helper Methods
	void addWhereOption(const QString& key, const QString& option, const QVariant& object);
	QNetworkRequest buildDefaultNetworkRequest();

	// Instance members
	QString				_className;
	QVariantMap			_whereMap;
	QSet<QString>		_whereEqualKeys;
	QStringList			_orderKeys;
	QSet<QString>		_includeKeys;
	QSet<QString>		_selectKeys;
	int					_limit;
	int					_skip;
	int					_count;
	QNetworkReply*		_getObjectReply;
	QNetworkReply*		_findReply;
	QNetworkReply*		_getFirstObjectReply;
	QNetworkReply*		_countReply;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFQueryPtr)

#endif	// End of PARSE_PFQUERY_H
