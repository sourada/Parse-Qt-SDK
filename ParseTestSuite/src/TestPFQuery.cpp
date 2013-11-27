//
//  TestPFQuery.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFQuery.h"
#include "TestRunner.h"

using namespace parse;

class TestPFQuery : public QObject
{
    Q_OBJECT

private slots:

	// Class init and cleanup methods
	void initTestCase() {}
	void cleanupTestCase() {}

	// Function init and cleanup methods (called before/after each test)
	void init() {}
	void cleanup() {}

	// Creation Methods
	void test_first();
};

void TestPFQuery::test_first()
{
	QCOMPARE(true, true);
}

DECLARE_TEST(TestPFQuery)
#include "TestPFQuery.moc"
