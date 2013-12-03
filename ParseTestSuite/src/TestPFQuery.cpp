//
//  TestPFQuery.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFError.h"
#include "PFObject.h"
#include "PFQuery.h"
#include "PFUser.h"
#include "TestRunner.h"

using namespace parse;

class TestPFQuery : public QObject
{
    Q_OBJECT

public slots:

	void getObjectCompleted(PFObjectPtr object, PFErrorPtr error)
	{
		_getObject = object;
		_getObjectError = error;
		emit getObjectEnded();
	}

	void findObjectsCompleted(PFObjectList objects, PFErrorPtr error)
	{
		_findObjects = objects;
		_findObjectsError = error;
		emit findObjectsEnded();
	}

	void getFirstObjectCompleted(PFObjectPtr object, PFErrorPtr error)
	{
		_getFirstObject = object;
		_getFirstObjectError = error;
		emit getFirstObjectEnded();
	}

	void countObjectsCompleted(int count, PFErrorPtr error)
	{
		_objectCount = count;
		_objectCountError = error;
		emit countObjectsEnded();
	}

signals:

	void getObjectEnded();
	void findObjectsEnded();
	void getFirstObjectEnded();
	void countObjectsEnded();

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		// Create some officials
		_umpire = PFObject::objectWithClassName("Official");
		_umpire->setObjectForKey(QString("Umpire"), "name");
		_umpire->setObjectForKey(QString("Baseball"), "sport");
		_objects.append(_umpire);

		PFObjectPtr lineJudge = PFObject::objectWithClassName("Official");
		lineJudge->setObjectForKey(QString("Line Judge"), "name");
		lineJudge->setObjectForKey(QString("Football"), "sport");
		_objects.append(lineJudge);

		PFObjectPtr referee = PFObject::objectWithClassName("Official");
		referee->setObjectForKey(QString("Referee"), "name");
		referee->setObjectForKey(QString("Basketball"), "sport");
		_objects.append(referee);

		// Create some objects to query for
		_baseball = PFObject::objectWithClassName("Sport");
		_baseball->setObjectForKey(QString("Baseball"), "name");
		_baseball->setObjectForKey(QString("Inning"), "timeSegment");
		_baseball->setObjectForKey(18, "totalPlayers");
		_baseball->setObjectForKey(_umpire, "official");
		_objects.append(_baseball);

		PFObjectPtr football = PFObject::objectWithClassName("Sport");
		football->setObjectForKey(QString("Football"), "name");
		football->setObjectForKey(QString("Quarter"), "timeSegment");
		football->setObjectForKey(22, "totalPlayers");
		football->setObjectForKey(lineJudge, "official");
		_objects.append(football);

		PFObjectPtr basketball = PFObject::objectWithClassName("Sport");
		basketball->setObjectForKey(QString("Basketball"), "name");
		basketball->setObjectForKey(QString("Period"), "timeSegment");
		basketball->setObjectForKey(10, "totalPlayers");
		basketball->setObjectForKey(referee, "official");
		_objects.append(basketball);

		// Save the objects in the cloud
		foreach (PFObjectPtr object, _objects)
			QCOMPARE(object->save(), true);
	}

	void cleanupTestCase()
	{
		// Delete the objects in the cloud
		foreach (PFObjectPtr object, _objects)
			QCOMPARE(object->deleteObject(), true);
	}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		// No-op
	}

	void cleanup()
	{
		_getObject = PFObjectPtr();
		_getObjectError = PFErrorPtr();
		_findObjects.clear();
		_findObjectsError = PFErrorPtr();
		_objectCount = -1;
		_objectCountError = PFErrorPtr();
	}

	// Creation Methods
	void test_queryWithClassName();

	// Key Inclusion/Exclusion
	void test_includeKey();
	void test_selectKeys();

	// Key Constraints - Object Comparisons
	void test_whereKeyEqualTo();
	void test_whereKeyNotEqualTo();

	// Sorting Methods
	void test_orderByAscending();
	void test_orderByDescending();
	void test_addAscendingOrder();
	void test_addDescendingOrder();

	// Pagination Methods
	void test_setLimit();
	void test_limit();
	void test_setSkip();
	void test_skip();

	// Get Object Methods
	void test_getObjectOfClassWithId();
	void test_getObjectOfClassWithIdWithError();
	void test_getObjectWithId();
	void test_getObjectWithIdWithError();
	void test_getObjectWithIdInBackground();

	// Get User Methods
	void test_getUserWithId();
	void test_getUserWithIdWithError();

	// Find Objects Methods
	void test_findObjects();
	void test_findObjectsWithError();
	void test_findObjectsInBackground();

	// Get First Object Methods
	void test_getFirstObject();
	void test_getFirstObjectWithError();
	void test_getFirstObjectInBackground();

	// Count Objects Methods
	void test_countObjects();
	void test_countObjectsWithError();
	void test_countObjectsInBackground();

	// Cancel Methods
	void test_cancel();

	// Accessor Methods
	void test_className();

private:

	// Instance members
	PFObjectList	_objects;
	PFObjectPtr		_baseball;
	PFObjectPtr		_umpire;
	PFObjectPtr		_getObject;
	PFErrorPtr		_getObjectError;
	PFObjectList	_findObjects;
	PFErrorPtr		_findObjectsError;
	PFObjectPtr		_getFirstObject;
	PFErrorPtr		_getFirstObjectError;
	int				_objectCount;
	PFErrorPtr		_objectCountError;
};

void TestPFQuery::test_queryWithClassName()
{
	// Invalid Case - empty className
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("");
	QCOMPARE(invalidQuery.isNull(), true);

	// Valid Case
	PFQueryPtr query = PFQuery::queryWithClassName("Character");
	QCOMPARE(query.isNull(), false);
	QCOMPARE(query->className(), QString("Character"));
}

void TestPFQuery::test_includeKey()
{
	// Query for the baseball object
	PFObjectPtr baseball = PFQuery::getObjectOfClassWithId("Sport", _baseball->objectId());
	QCOMPARE(baseball.isNull(), false);

	// Extract the umpire and verify that the object was not included
	PFObjectPtr umpire = PFObject::objectFromVariant(baseball->objectForKey("official"));
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->allKeys().contains("name"), false);
	QCOMPARE(umpire->allKeys().contains("sport"), false);

	// Fetch the umpire to ensure we can pull the keys down
	QCOMPARE(umpire->fetch(), true);
	QCOMPARE(umpire->allKeys().contains("name"), true);
	QCOMPARE(umpire->allKeys().contains("sport"), true);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));

	// Query for the baseball object again except use the include key to pull the umpire as well
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->includeKey("official");
	baseball = query->getObjectWithId(_baseball->objectId());
	QCOMPARE(baseball.isNull(), false);

	// Extract the umpire and verify that the object WAS included
	umpire = PFObject::objectFromVariant(baseball->objectForKey("official"));
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->allKeys().contains("name"), true);
	QCOMPARE(umpire->allKeys().contains("sport"), true);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_selectKeys()
{
	// Query for the baseball object and restrict it to the name and totalPlayers keys
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	QStringList keys;
	keys << "name" << "totalPlayers";
	query->selectKeys(keys);
	PFObjectPtr baseball = query->getObjectWithId(_baseball->objectId());
	QCOMPARE(baseball.isNull(), false);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->allKeys().count(), 2);
	QCOMPARE(baseball->allKeys().contains("name"), true);
	QCOMPARE(baseball->allKeys().contains("totalPlayers"), true);
	QCOMPARE(baseball->allKeys().contains("timeSegment"), false);
	QCOMPARE(baseball->allKeys().contains("official"), false);
}

void TestPFQuery::test_whereKeyEqualTo()
{
	// Create a query with one keyEqualTo constraint
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->whereKeyEqualTo("name", QString("Baseball"));
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.length(), 1);
	PFObjectPtr baseball = objects.at(0);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball->objectForKey("totalPlayers").toInt(), 18);

	// Create a query with multiple keyEqualTo constraints (no matching objects)
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->whereKeyEqualTo("name", QString("Baseball"));
	query2->whereKeyEqualTo("timeSegment", QString("Quarter"));
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.length(), 0);

	// Create a query with multiple keyEqualTo constraints (should be one matching object)
	PFQueryPtr query3 = PFQuery::queryWithClassName("Sport");
	query3->whereKeyEqualTo("name", QString("Football"));
	query3->whereKeyEqualTo("totalPlayers", 22);
	PFObjectList objects3 = query3->findObjects();
	QCOMPARE(objects3.length(), 1);
	PFObjectPtr football = objects3.at(0);
	QCOMPARE(football->className(), QString("Sport"));
	QCOMPARE(football->objectId().isEmpty(), false);
	QCOMPARE(football->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(football->objectForKey("timeSegment").toString(), QString("Quarter"));
	QCOMPARE(football->objectForKey("totalPlayers").toInt(), 22);

	// Query with PFObject
	PFQueryPtr query4 = PFQuery::queryWithClassName("Sport");
	query4->whereKeyEqualTo("official", PFObject::toVariant(_umpire));
	PFObjectList objects4 = query4->findObjects();
	QCOMPARE(objects4.length(), 1);
	PFObjectPtr baseball2 = objects4.at(0);
	QCOMPARE(baseball2->className(), QString("Sport"));
	QCOMPARE(baseball2->objectId().isEmpty(), false);
	QCOMPARE(baseball2->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball2->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball2->objectForKey("totalPlayers").toInt(), 18);
}

void TestPFQuery::test_whereKeyNotEqualTo()
{
	// Create a query with one keyNotEqualTo constraint
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->whereKeyNotEqualTo("name", QString("Baseball"));
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.length(), 2);
	foreach (PFObjectPtr object, objects)
	{
		QCOMPARE(object->className(), QString("Sport"));
		QCOMPARE(object->objectId().isEmpty(), false);
		QCOMPARE(object->objectForKey("name").toString().isEmpty(), false);
		QCOMPARE(object->objectForKey("timeSegment").toString().isEmpty(), false);
		QCOMPARE(object->objectForKey("totalPlayers").toInt() > 0, true);
	}

	// Create a query with multiple keyNotEqualTo constraints (no matching objects)
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->whereKeyNotEqualTo("name", QString("Baseball"));
	query2->whereKeyNotEqualTo("timeSegment", QString("Quarter"));
	query2->whereKeyNotEqualTo("totalPlayers", 10);
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.length(), 0);

	// Create a query with multiple keyNotEqualTo constraints (should be one matching object)
	PFQueryPtr query3 = PFQuery::queryWithClassName("Sport");
	query3->whereKeyNotEqualTo("name", QString("Football"));
	query3->whereKeyNotEqualTo("totalPlayers", 10);
	PFObjectList objects3 = query3->findObjects();
	QCOMPARE(objects3.length(), 1);
	PFObjectPtr baseball = objects3.at(0);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball->objectForKey("totalPlayers").toInt(), 18);

	// Query with PFObject
	PFQueryPtr query4 = PFQuery::queryWithClassName("Sport");
	query4->whereKeyNotEqualTo("official", PFObject::toVariant(_umpire));
	PFObjectList objects4 = query4->findObjects();
	QCOMPARE(objects4.length(), 2);
	foreach (PFObjectPtr object, objects4)
	{
		QCOMPARE(object->className(), QString("Sport"));
		QCOMPARE(object->objectId().isEmpty(), false);
		QCOMPARE(object->objectForKey("name").toString().isEmpty(), false);
		QCOMPARE(object->objectForKey("timeSegment").toString().isEmpty(), false);
		QCOMPARE(object->objectForKey("totalPlayers").toInt() > 0, true);
	}
}

void TestPFQuery::test_orderByAscending()
{
	// Sort by name
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->orderByAscending("name");
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 3);
	QCOMPARE(objects.at(0)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects.at(1)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects.at(2)->objectForKey("name").toString(), QString("Football"));

	// Sort by totalPlayers
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->orderByAscending("totalPlayers");
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.count(), 3);
	QCOMPARE(objects2.at(0)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects2.at(1)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects2.at(2)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects2.at(0)->objectForKey("totalPlayers").toInt(), 10);
	QCOMPARE(objects2.at(1)->objectForKey("totalPlayers").toInt(), 18);
	QCOMPARE(objects2.at(2)->objectForKey("totalPlayers").toInt(), 22);
}

void TestPFQuery::test_orderByDescending()
{
	// Sort by name
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->orderByDescending("name");
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 3);
	QCOMPARE(objects.at(0)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects.at(1)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects.at(2)->objectForKey("name").toString(), QString("Baseball"));

	// Sort by totalPlayers
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->orderByDescending("totalPlayers");
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.count(), 3);
	QCOMPARE(objects2.at(0)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects2.at(1)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects2.at(2)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects2.at(0)->objectForKey("totalPlayers").toInt(), 22);
	QCOMPARE(objects2.at(1)->objectForKey("totalPlayers").toInt(), 18);
	QCOMPARE(objects2.at(2)->objectForKey("totalPlayers").toInt(), 10);
}

void TestPFQuery::test_addAscendingOrder()
{
	// Sort by name (will ignore totalPlayers since no key values are the same)
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->addAscendingOrder("name");
	query->addAscendingOrder("totalPlayers");
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 3);
	QCOMPARE(objects.at(0)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects.at(1)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects.at(2)->objectForKey("name").toString(), QString("Football"));

	// Sort by totalPlayers (will ignore name since no key values are the same)
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->addAscendingOrder("totalPlayers");
	query2->addAscendingOrder("name");
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.count(), 3);
	QCOMPARE(objects2.at(0)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects2.at(1)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects2.at(2)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects2.at(0)->objectForKey("totalPlayers").toInt(), 10);
	QCOMPARE(objects2.at(1)->objectForKey("totalPlayers").toInt(), 18);
	QCOMPARE(objects2.at(2)->objectForKey("totalPlayers").toInt(), 22);
}

void TestPFQuery::test_addDescendingOrder()
{
	// Sort by name (will ignore totalPlayers since no key values are the same)
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->addDescendingOrder("name");
	query->addDescendingOrder("totalPlayers");
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 3);
	QCOMPARE(objects.at(0)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects.at(1)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects.at(2)->objectForKey("name").toString(), QString("Baseball"));

	// Sort by totalPlayers (will ignore name since no key values are the same)
	PFQueryPtr query2 = PFQuery::queryWithClassName("Sport");
	query2->addDescendingOrder("totalPlayers");
	query2->addDescendingOrder("name");
	PFObjectList objects2 = query2->findObjects();
	QCOMPARE(objects2.count(), 3);
	QCOMPARE(objects2.at(0)->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(objects2.at(1)->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(objects2.at(2)->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(objects2.at(0)->objectForKey("totalPlayers").toInt(), 22);
	QCOMPARE(objects2.at(1)->objectForKey("totalPlayers").toInt(), 18);
	QCOMPARE(objects2.at(2)->objectForKey("totalPlayers").toInt(), 10);
}

void TestPFQuery::test_setLimit()
{
	// Query for sports but set the limit to 0
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->setLimit(0);
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 0);

	// Query for sports but set the limit to 2
	query = PFQuery::queryWithClassName("Sport");
	query->setLimit(2);
	objects = query->findObjects();
	QCOMPARE(objects.count(), 2);
}

void TestPFQuery::test_limit()
{
	// Try some different combos (simple b/c it is only a getter)
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	QCOMPARE(query->limit(), -1);
	query->setLimit(0);
	QCOMPARE(query->limit(), 0);
	query->setLimit(2);
	QCOMPARE(query->limit(), 2);
}

void TestPFQuery::test_setSkip()
{
	// Query for sports but skip the first 2
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->setSkip(2);
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.count(), 1);

	// Query for officials but skip the first 3
	query = PFQuery::queryWithClassName("Official");
	query->setSkip(3);
	objects = query->findObjects();
	QCOMPARE(objects.count(), 0);
}

void TestPFQuery::test_skip()
{
	// Try some different combos (simple b/c it is only a getter)
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	QCOMPARE(query->skip(), -1);
	query->setSkip(0);
	QCOMPARE(query->skip(), 0);
	query->setSkip(2);
	QCOMPARE(query->skip(), 2);
}

void TestPFQuery::test_getObjectOfClassWithId()
{
	// Invalid Case - query for an object with no class name or object id
	PFObjectPtr invalidObject = PFQuery::getObjectOfClassWithId("", "");
	QCOMPARE(invalidObject.isNull(), true);

	// Valid Case 1 - query for an object with a valid class name and empty object id
	PFObjectPtr emptyObject = PFQuery::getObjectOfClassWithId("Sport", "");
	QCOMPARE(emptyObject.isNull(), true);

	// Valid Case 2 - query for an object that actually exists
	PFObjectPtr umpire = PFQuery::getObjectOfClassWithId("Official", _umpire->objectId());
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_getObjectOfClassWithIdWithError()
{
	// Invalid Case - query for an object with no class name or object id
	PFErrorPtr invalidObjectError;
	PFObjectPtr invalidObject = PFQuery::getObjectOfClassWithId("", "", invalidObjectError);
	QCOMPARE(invalidObject.isNull(), true);
	QCOMPARE(invalidObjectError.isNull(), true);

	// Valid Case 1 - query for an object with a valid class name and empty object id
	PFErrorPtr emptyObjectError;
	PFObjectPtr emptyObject = PFQuery::getObjectOfClassWithId("Sport", "", emptyObjectError);
	QCOMPARE(emptyObject.isNull(), true);
	QCOMPARE(emptyObjectError.isNull(), true);

	// Valid Case 1 - query for an object that actually exists
	PFErrorPtr umpireError;
	PFObjectPtr umpire = PFQuery::getObjectOfClassWithId("Official", _umpire->objectId(), umpireError);
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpireError.isNull(), true);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_getObjectWithId()
{
	// Invalid Case - query for a class that does exist with an empty object id
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	PFObjectPtr invalidObject = invalidQuery->getObjectWithId("");
	QCOMPARE(invalidObject.isNull(), true);

	// Valid Case - no matching object for the empty object id
	PFQueryPtr emptyQuery = PFQuery::queryWithClassName("Sport");
	PFObjectPtr emptyObject = emptyQuery->getObjectWithId("");
	QCOMPARE(emptyObject.isNull(), true);

	// Valid Case - get the umpire object
	PFQueryPtr query = PFQuery::queryWithClassName("Official");
	PFObjectPtr umpire = query->getObjectWithId(_umpire->objectId());
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_getObjectWithIdWithError()
{
	// Invalid Case - query for a class that does exist with an empty object id
	PFErrorPtr invalidQueryError;
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	PFObjectPtr invalidObject = invalidQuery->getObjectWithId("", invalidQueryError);
	QCOMPARE(invalidObject.isNull(), true);
	QCOMPARE(invalidQueryError.isNull(), true);

	// Valid Case - no matching object for the empty object id
	PFErrorPtr emptyQueryError;
	PFQueryPtr emptyQuery = PFQuery::queryWithClassName("Sport");
	PFObjectPtr emptyObject = emptyQuery->getObjectWithId("", emptyQueryError);
	QCOMPARE(emptyObject.isNull(), true);
	QCOMPARE(emptyQueryError.isNull(), true);

	// Valid Case - get the umpire object
	PFErrorPtr queryError;
	PFQueryPtr query = PFQuery::queryWithClassName("Official");
	PFObjectPtr umpire = query->getObjectWithId(_umpire->objectId(), queryError);
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(queryError.isNull(), true);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_getObjectWithIdInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;
	QObject::connect(this, SIGNAL(getObjectEnded()), &eventLoop, SLOT(quit()));

	// Invalid Case - query for a class that does exist with an empty object id
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	invalidQuery->getObjectWithIdInBackground("", this, SLOT(getObjectCompleted(PFObjectPtr, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_getObject.isNull(), true);
	QCOMPARE(_getObjectError.isNull(), true);

	// Reset callback flags
	_getObject = PFObjectPtr();
	_getObjectError = PFErrorPtr();

	// Valid Case - no matching object for the empty object id
	PFQueryPtr emptyQuery = PFQuery::queryWithClassName("Sport");
	emptyQuery->getObjectWithIdInBackground("", this, SLOT(getObjectCompleted(PFObjectPtr, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_getObject.isNull(), true);
	QCOMPARE(_getObjectError.isNull(), true);

	// Reset callback flags
	_getObject = PFObjectPtr();
	_getObjectError = PFErrorPtr();

	// Valid Case - get the umpire object
	PFQueryPtr query = PFQuery::queryWithClassName("Official");
	query->getObjectWithIdInBackground(_umpire->objectId(), this, SLOT(getObjectCompleted(PFObjectPtr, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_getObjectError.isNull(), true);
	PFObjectPtr umpire = _getObject;
	QCOMPARE(umpire.isNull(), false);
	QCOMPARE(umpire->className(), QString("Official"));
	QCOMPARE(umpire->objectId().isEmpty(), false);
	QCOMPARE(umpire->objectForKey("name").toString(), QString("Umpire"));
	QCOMPARE(umpire->objectForKey("sport").toString(), QString("Baseball"));
}

void TestPFQuery::test_getUserWithId()
{
	// Create a test user
	PFUserPtr user = PFUser::user();
	user->setUsername("test_getUserWithId");
	user->setEmail("test_getUserWithId@parse.com");
	user->setPassword("testPassword");
	user->setObjectForKey(QString("123-456-7890"), "phone");
	QCOMPARE(PFUser::signUpWithUser(user), true);
	QCOMPARE(user->objectId().isEmpty(), false);

	// Query for a user with a blank object id
	PFUserPtr cloudUser = PFQuery::getUserWithId("");
	QCOMPARE(cloudUser.isNull(), true);

	// Query for the user using the object id
	cloudUser = PFQuery::getUserWithId(user->objectId());
	QCOMPARE(cloudUser.isNull(), false);
	QCOMPARE(cloudUser->username(), user->username());
	QCOMPARE(cloudUser->email(), user->email());
	QCOMPARE(cloudUser->password().isEmpty(), true);
	QCOMPARE(cloudUser->objectForKey("phone"), user->objectForKey("phone"));

	// Cleanup
	QCOMPARE(user->deleteObject(), true);
}

void TestPFQuery::test_getUserWithIdWithError()
{
	// Create a test user
	PFUserPtr user = PFUser::user();
	user->setUsername("test_getUserWithIdWithError");
	user->setEmail("test_getUserWithIdWithError@parse.com");
	user->setPassword("testPassword");
	user->setObjectForKey(QString("123-456-7890"), "phone");
	QCOMPARE(PFUser::signUpWithUser(user), true);
	QCOMPARE(user->objectId().isEmpty(), false);

	// Query for a user with a blank object id
	PFErrorPtr queryError;
	PFUserPtr cloudUser = PFQuery::getUserWithId("", queryError);
	QCOMPARE(cloudUser.isNull(), true);
	QCOMPARE(queryError.isNull(), true);

	// Query for the user using the object id
	cloudUser = PFQuery::getUserWithId(user->objectId(), queryError);
	QCOMPARE(cloudUser.isNull(), false);
	QCOMPARE(queryError.isNull(), true);
	QCOMPARE(cloudUser->username(), user->username());
	QCOMPARE(cloudUser->email(), user->email());
	QCOMPARE(cloudUser->password().isEmpty(), true);
	QCOMPARE(cloudUser->objectForKey("phone"), user->objectForKey("phone"));

	// Cleanup
	QCOMPARE(user->deleteObject(), true);
}

void TestPFQuery::test_findObjects()
{
	// Invalid Case - query for a class that does exist
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	PFObjectList invalidObjects = invalidQuery->findObjects();
	QCOMPARE(invalidObjects.length(), 0);

	// Valid Case
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	PFObjectList objects = query->findObjects();
	QCOMPARE(objects.length(), 3);

	// Baseball
	PFObjectPtr baseball = objects.at(0);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball->objectForKey("totalPlayers").toInt(), 18);

	// Football
	PFObjectPtr football = objects.at(1);
	QCOMPARE(football->className(), QString("Sport"));
	QCOMPARE(football->objectId().isEmpty(), false);
	QCOMPARE(football->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(football->objectForKey("timeSegment").toString(), QString("Quarter"));
	QCOMPARE(football->objectForKey("totalPlayers").toInt(), 22);

	// Basketball
	PFObjectPtr basketball = objects.at(2);
	QCOMPARE(basketball->className(), QString("Sport"));
	QCOMPARE(basketball->objectId().isEmpty(), false);
	QCOMPARE(basketball->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(basketball->objectForKey("timeSegment").toString(), QString("Period"));
	QCOMPARE(basketball->objectForKey("totalPlayers").toInt(), 10);
}

void TestPFQuery::test_findObjectsWithError()
{
	// Invalid Case - query for a class that does exist
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	PFErrorPtr invalidQueryError;
	PFObjectList invalidObjects = invalidQuery->findObjects(invalidQueryError);
	QCOMPARE(invalidObjects.length(), 0);
	QCOMPARE(invalidQueryError.isNull(), true);

	// Valid Case
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	PFErrorPtr queryError;
	PFObjectList objects = query->findObjects(queryError);
	QCOMPARE(objects.length(), 3);
	QCOMPARE(queryError.isNull(), true);

	// Baseball
	PFObjectPtr baseball = objects.at(0);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball->objectForKey("totalPlayers").toInt(), 18);

	// Football
	PFObjectPtr football = objects.at(1);
	QCOMPARE(football->className(), QString("Sport"));
	QCOMPARE(football->objectId().isEmpty(), false);
	QCOMPARE(football->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(football->objectForKey("timeSegment").toString(), QString("Quarter"));
	QCOMPARE(football->objectForKey("totalPlayers").toInt(), 22);

	// Basketball
	PFObjectPtr basketball = objects.at(2);
	QCOMPARE(basketball->className(), QString("Sport"));
	QCOMPARE(basketball->objectId().isEmpty(), false);
	QCOMPARE(basketball->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(basketball->objectForKey("timeSegment").toString(), QString("Period"));
	QCOMPARE(basketball->objectForKey("totalPlayers").toInt(), 10);
}

void TestPFQuery::test_findObjectsInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;
	QObject::connect(this, SIGNAL(findObjectsEnded()), &eventLoop, SLOT(quit()));

	// Invalid Case - query for a class that does exist
	PFQueryPtr invalidQuery = PFQuery::queryWithClassName("TheresNoPossibleWayToGetMe");
	invalidQuery->findObjectsInBackground(this, SLOT(findObjectsCompleted(PFObjectList, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_findObjects.length(), 0);
	QCOMPARE(_findObjectsError.isNull(), true);

	// Reset callback flags
	_findObjects.clear();
	_findObjectsError = PFErrorPtr();

	// Valid Case
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	PFObjectList objects = query->findObjects();
	query->findObjectsInBackground(this, SLOT(findObjectsCompleted(PFObjectList, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_findObjects.length(), 3);
	QCOMPARE(_findObjectsError.isNull(), true);

	// Baseball
	PFObjectPtr baseball = objects.at(0);
	QCOMPARE(baseball->className(), QString("Sport"));
	QCOMPARE(baseball->objectId().isEmpty(), false);
	QCOMPARE(baseball->objectForKey("name").toString(), QString("Baseball"));
	QCOMPARE(baseball->objectForKey("timeSegment").toString(), QString("Inning"));
	QCOMPARE(baseball->objectForKey("totalPlayers").toInt(), 18);

	// Football
	PFObjectPtr football = objects.at(1);
	QCOMPARE(football->className(), QString("Sport"));
	QCOMPARE(football->objectId().isEmpty(), false);
	QCOMPARE(football->objectForKey("name").toString(), QString("Football"));
	QCOMPARE(football->objectForKey("timeSegment").toString(), QString("Quarter"));
	QCOMPARE(football->objectForKey("totalPlayers").toInt(), 22);

	// Basketball
	PFObjectPtr basketball = objects.at(2);
	QCOMPARE(basketball->className(), QString("Sport"));
	QCOMPARE(basketball->objectId().isEmpty(), false);
	QCOMPARE(basketball->objectForKey("name").toString(), QString("Basketball"));
	QCOMPARE(basketball->objectForKey("timeSegment").toString(), QString("Period"));
	QCOMPARE(basketball->objectForKey("totalPlayers").toInt(), 10);
}

void TestPFQuery::test_getFirstObject()
{
	// Get the first sport
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	PFObjectPtr sport = query->getFirstObject();
	QCOMPARE(sport.isNull(), false);
	QCOMPARE(sport->objectId().isEmpty(), false);
	QCOMPARE(sport->className(), QString("Sport"));
	QCOMPARE(sport->objectForKey("timeSegment").toString().isEmpty(), false);

	// Get the first official
	query = PFQuery::queryWithClassName("Official");
	PFObjectPtr official = query->getFirstObject();
	QCOMPARE(official.isNull(), false);
	QCOMPARE(official->objectId().isEmpty(), false);
	QCOMPARE(official->className(), QString("Official"));
	QCOMPARE(official->objectForKey("sport").toString().isEmpty(), false);
}

void TestPFQuery::test_getFirstObjectWithError()
{
	// Get the first sport
	PFErrorPtr queryError;
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	PFObjectPtr sport = query->getFirstObject(queryError);
	QCOMPARE(sport.isNull(), false);
	QCOMPARE(queryError.isNull(), true);
	QCOMPARE(sport->objectId().isEmpty(), false);
	QCOMPARE(sport->className(), QString("Sport"));
	QCOMPARE(sport->objectForKey("timeSegment").toString().isEmpty(), false);

	// Get the first official
	query = PFQuery::queryWithClassName("Official");
	PFObjectPtr official = query->getFirstObject(queryError);
	QCOMPARE(official.isNull(), false);
	QCOMPARE(queryError.isNull(), true);
	QCOMPARE(official->objectId().isEmpty(), false);
	QCOMPARE(official->className(), QString("Official"));
	QCOMPARE(official->objectForKey("sport").toString().isEmpty(), false);
}

void TestPFQuery::test_getFirstObjectInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;
	QObject::connect(this, SIGNAL(getFirstObjectEnded()), &eventLoop, SLOT(quit()));

	// Get the first sport
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->getFirstObjectInBackground(this, SLOT(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	PFObjectPtr sport = _getFirstObject;
	QCOMPARE(sport.isNull(), false);
	QCOMPARE(_getFirstObjectError.isNull(), true);
	QCOMPARE(sport->objectId().isEmpty(), false);
	QCOMPARE(sport->className(), QString("Sport"));
	QCOMPARE(sport->objectForKey("timeSegment").toString().isEmpty(), false);

	// Reset callback flags
	_getFirstObject = PFObjectPtr();
	_getFirstObjectError = PFErrorPtr();

	// Get the first official
	query = PFQuery::queryWithClassName("Official");
	query->getFirstObjectInBackground(this, SLOT(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	PFObjectPtr official = _getFirstObject;
	QCOMPARE(official.isNull(), false);
	QCOMPARE(_getFirstObjectError.isNull(), true);
	QCOMPARE(official->objectId().isEmpty(), false);
	QCOMPARE(official->className(), QString("Official"));
	QCOMPARE(official->objectForKey("sport").toString().isEmpty(), false);
}

void TestPFQuery::test_countObjects()
{
	// Simple count query
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	QCOMPARE(query->countObjects(), 3);

	// More complex query
	query = PFQuery::queryWithClassName("Official");
	query->whereKeyNotEqualTo("name", QString("Umpire"));
	query->whereKeyNotEqualTo("sport", QString("Football"));
	query->orderByAscending("name");
	QCOMPARE(query->countObjects(), 1);
}

void TestPFQuery::test_countObjectsWithError()
{
	// Simple count query
	PFErrorPtr queryError;
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	QCOMPARE(query->countObjects(queryError), 3);
	QCOMPARE(queryError.isNull(), true);

	// More complex query
	query = PFQuery::queryWithClassName("Official");
	query->whereKeyNotEqualTo("name", QString("Umpire"));
	query->whereKeyNotEqualTo("sport", QString("Football"));
	query->orderByAscending("name");
	QCOMPARE(query->countObjects(queryError), 1);
	QCOMPARE(queryError.isNull(), true);
}

void TestPFQuery::test_countObjectsInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;
	QObject::connect(this, SIGNAL(countObjectsEnded()), &eventLoop, SLOT(quit()));

	// Simple count query
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->countObjectsInBackground(this, SLOT(countObjectsCompleted(int, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_objectCount, 3);
	QCOMPARE(_objectCountError.isNull(), true);

	// Reset callback flags
	_objectCount = -1;
	_objectCountError = PFErrorPtr();

	// More complex query
	query = PFQuery::queryWithClassName("Official");
	query->whereKeyNotEqualTo("name", QString("Umpire"));
	query->whereKeyNotEqualTo("sport", QString("Football"));
	query->orderByAscending("name");
	query->countObjectsInBackground(this, SLOT(countObjectsCompleted(int, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_objectCount, 1);
	QCOMPARE(_objectCountError.isNull(), true);
}

void TestPFQuery::test_cancel()
{
	// Create a query and cancel it (should just return)
	PFQueryPtr query = PFQuery::queryWithClassName("Sport");
	query->cancel();

	// Start up a background get object query and cancel it.
	query = PFQuery::queryWithClassName("Official");
	query->getObjectWithIdInBackground("", this, SLOT(getObjectCompleted(PFObjectPtr, PFErrorPtr)));
	query->cancel();

	// Start up a background find query and cancel it.
	query = PFQuery::queryWithClassName("Sport");
	query->findObjectsInBackground(this, SLOT(findObjectsCompleted(PFObjectList, PFErrorPtr)));
	query->cancel();

	// Start up a background get first object query and cancel it.
	query = PFQuery::queryWithClassName("Official");
	query->getFirstObjectInBackground(this, SLOT(getFirstObjectCompleted(PFObjectPtr, PFErrorPtr)));
	query->cancel();

	// Start up a background count query and cancel it.
	query = PFQuery::queryWithClassName("Sport");
	query->countObjectsInBackground(this, SLOT(countObjectsCompleted(int, PFErrorPtr)));
	query->cancel();
}

void TestPFQuery::test_className()
{
	PFQueryPtr query = PFQuery::queryWithClassName("Character");
	QCOMPARE(query.isNull(), false);
	QCOMPARE(query->className(), QString("Character"));
}

DECLARE_TEST(TestPFQuery)
#include "TestPFQuery.moc"
