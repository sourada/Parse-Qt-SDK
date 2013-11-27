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
		// Create some objects to query for
		PFObjectPtr baseball = PFObject::objectWithClassName("Sport");
		baseball->setObjectForKey(QString("Baseball"), "name");
		baseball->setObjectForKey(QString("Inning"), "timeSegment");
		baseball->setObjectForKey(18, "totalPlayers");
		_objects.append(baseball);

		PFObjectPtr football = PFObject::objectWithClassName("Sport");
		football->setObjectForKey(QString("Football"), "name");
		football->setObjectForKey(QString("Quarter"), "timeSegment");
		football->setObjectForKey(22, "totalPlayers");
		_objects.append(football);

		PFObjectPtr basketball = PFObject::objectWithClassName("Sport");
		basketball->setObjectForKey(QString("Basketball"), "name");
		basketball->setObjectForKey(QString("Period"), "timeSegment");
		basketball->setObjectForKey(10, "totalPlayers");
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

	// Find Objects Methods
	void test_findObjects();
	void test_findObjectsWithError();
	void test_findObjectsInBackground();

	// Accessor Methods
	void test_className();

private:

	// Instance members
	PFObjectList	_objects;
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
