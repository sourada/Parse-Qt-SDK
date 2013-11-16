//
//  TestPFSerializable.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/14/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFObject.h"
#include "PFSerializable.h"
#include "TestRunner.h"

using namespace parse;

class TestPFSerializable : public QObject
{
    Q_OBJECT

private slots:

	// Class init and cleanup methods
	void initTestCase() {}
	void cleanupTestCase() {}

	// Function init and cleanup methods (called before/after each test)
	void init() {}
	void cleanup() {}

	// Variant Conversion Helpers
	void test_toVariant();
};

void TestPFSerializable::test_toVariant()
{
	// Valid Case - Empty Pointer
	QVariant emptyVariant = PFSerializable::toVariant(PFSerializablePtr());
	QCOMPARE(emptyVariant.isNull(), false);
	QCOMPARE(emptyVariant.canConvert<PFSerializablePtr>(), true);

	// Valid Case - PFObject
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QVariant levelVariant = PFSerializable::toVariant(level);
	QCOMPARE(levelVariant.isNull(), false);
	QCOMPARE(levelVariant.canConvert<PFSerializablePtr>(), true);
}

DECLARE_TEST(TestPFSerializable)
#include "TestPFSerializable.moc"
