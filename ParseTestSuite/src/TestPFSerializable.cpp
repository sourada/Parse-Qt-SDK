//
//  TestPFSerializable.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/14/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
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
	void test_fromVariant();
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

void TestPFSerializable::test_fromVariant()
{
	// Invalid Case 1 - Empty Serializable
	QVariant emptyVariant;
	PFSerializablePtr emptySerializable = PFSerializable::fromVariant(emptyVariant);
	QCOMPARE(emptySerializable.isNull(), true);

	// Invalid Case 2 - QString
	QString testString = "test";
	QVariant testStringVariant = testString;
	PFSerializablePtr testStringSerializable = PFSerializable::fromVariant(testStringVariant);
	QCOMPARE(testStringSerializable.isNull(), true);

	// Valid Case - PFObject
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QVariant levelVariant = PFSerializable::toVariant(level);
	PFSerializablePtr serializableLevel = PFSerializable::fromVariant(levelVariant);
	QCOMPARE(serializableLevel.isNull(), false);
}

DECLARE_TEST(TestPFSerializable)
#include "TestPFSerializable.moc"
