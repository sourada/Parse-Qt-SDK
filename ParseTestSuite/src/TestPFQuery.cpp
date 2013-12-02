//
//  TestPFQuery.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFObject.h"
#include "PFQuery.h"
#include "TestRunner.h"

using namespace parse;

class TestPFQuery : public QObject
{
    Q_OBJECT

public slots:

	void findObjectsCompleted(PFObjectList objects, PFErrorPtr error)
	{
		_findObjects = objects;
		_findObjectsError = error;
		emit findObjectsEnded();
	}

signals:

	void findObjectsEnded();

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
		PFObjectPtr baseball = PFObject::objectWithClassName("Sport");
		baseball->setObjectForKey(QString("Baseball"), "name");
		baseball->setObjectForKey(QString("Inning"), "timeSegment");
		baseball->setObjectForKey(18, "totalPlayers");
		baseball->setObjectForKey(_umpire, "official");
		_objects.append(baseball);

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
		_findObjects.clear();
		_findObjectsError = PFErrorPtr();
	}

	// Creation Methods
	void test_queryWithClassName();

	// Key Constraint Methods
	void test_whereKeyEqualTo();
	void test_whereKeyNotEqualTo();

	// Find Objects Methods
	void test_findObjects();
	void test_findObjectsWithError();
	void test_findObjectsInBackground();

	// Accessor Methods
	void test_className();

private:

	// Instance members
	PFObjectList	_objects;
	PFObjectPtr		_umpire;
	PFObjectList	_findObjects;
	PFErrorPtr		_findObjectsError;
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
	PFObjectList invalidObjects = invalidQuery->findObjects();
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

void TestPFQuery::test_className()
{
	PFQueryPtr query = PFQuery::queryWithClassName("Character");
	QCOMPARE(query.isNull(), false);
	QCOMPARE(query->className(), QString("Character"));
}

DECLARE_TEST(TestPFQuery)
#include "TestPFQuery.moc"
