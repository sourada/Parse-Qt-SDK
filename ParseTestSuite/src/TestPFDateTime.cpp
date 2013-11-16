//
//  TestPFDateTime.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFDateTime.h"
#include "TestRunner.h"

using namespace parse;

class TestPFDateTime : public QObject
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
	void test_dateTimeFromParseString();
	void test_dateTimeFromDateTime();
	void test_dateTimeFromVariant();

	// DateTime Access
	void test_toParseString();
	void test_dateTime();

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_pfClassName();
};

void TestPFDateTime::test_dateTimeFromParseString()
{
	// Valid case
	QString parseDateString = "2013-09-15T09:32:00.123Z";
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromParseString(parseDateString);
	QDateTime dateTime = pfDateTime->dateTime();
	QCOMPARE(dateTime.timeSpec(), Qt::UTC);
	QCOMPARE(dateTime.date(), QDate(2013, 9, 15));
	QCOMPARE(dateTime.time(), QTime(9, 32, 0, 123));

	// Invalid parse string
	parseDateString = "2013-09-15 09:32:00.123";
	QCOMPARE(PFDateTime::dateTimeFromParseString(parseDateString), PFDateTimePtr());
}

void TestPFDateTime::test_dateTimeFromDateTime()
{
	// Valid case
	QDateTime dateTime(QDate(2013, 11, 12), QTime(13, 16, 47, 348));
	dateTime.setTimeSpec(Qt::UTC);
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromDateTime(dateTime);
	QDateTime actualDateTime = pfDateTime->dateTime();
	QCOMPARE(actualDateTime.timeSpec(), Qt::UTC);
	QCOMPARE(actualDateTime.date(), QDate(2013, 11, 12));
	QCOMPARE(actualDateTime.time(), QTime(13, 16, 47, 348));

	// Invalid case
	QDateTime emptyDate;
	QCOMPARE(PFDateTime::dateTimeFromDateTime(emptyDate), PFDateTimePtr());
}

void TestPFDateTime::test_dateTimeFromVariant()
{
	// Valid Case
	PFDateTimePtr dateTime = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	QVariant dateTimeVariant = PFDateTime::toVariant(dateTime);
	PFDateTimePtr convertedDateTime = PFDateTime::dateTimeFromVariant(dateTimeVariant);
	QCOMPARE(convertedDateTime.isNull(), false);
	QCOMPARE(convertedDateTime->dateTime().toLocalTime().date(), QDate::currentDate());

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFDateTimePtr convertedString = PFDateTime::dateTimeFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);
}

void TestPFDateTime::test_toParseString()
{
	QDateTime dateTime(QDate(2013, 11, 12), QTime(13, 16, 47, 348));
	dateTime.setTimeSpec(Qt::UTC);
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromDateTime(dateTime);
	QCOMPARE(pfDateTime->toParseString(), QString("2013-11-12T13:16:47.348Z"));
}

void TestPFDateTime::test_dateTime()
{
	QDateTime dateTime(QDate(2013, 11, 12), QTime(13, 16, 47, 348));
	dateTime.setTimeSpec(Qt::UTC);
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromDateTime(dateTime);
	QDateTime actualDateTime = pfDateTime->dateTime();
	QCOMPARE(actualDateTime.date(), QDate(2013, 11, 12));
	QCOMPARE(actualDateTime.time(), QTime(13, 16, 47, 348));
	QCOMPARE(actualDateTime.timeSpec(), Qt::UTC);
}

void TestPFDateTime::test_fromJson()
{
	// Create a date time to test
	QDateTime dateTime(QDate(2013, 11, 12), QTime(13, 16, 47, 348));
	dateTime.setTimeSpec(Qt::UTC);
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromDateTime(dateTime);

	// Convert the date time to json
	QJsonObject jsonObject;
	QCOMPARE(pfDateTime->toJson(jsonObject), true);

	// Convert the json back to a date time
	QVariant dateTimeVariant = PFDateTime::fromJson(jsonObject);
	PFDateTimePtr convertedDateTime = PFDateTime::dateTimeFromVariant(dateTimeVariant);

	// Test out the results of the conversion
	QCOMPARE(convertedDateTime.isNull(), false);
	QCOMPARE(convertedDateTime->toParseString(), pfDateTime->toParseString());
}

void TestPFDateTime::test_toJson()
{
	QDateTime dateTime(QDate(2013, 11, 12), QTime(13, 16, 47, 348));
	dateTime.setTimeSpec(Qt::UTC);
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromDateTime(dateTime);
	QJsonObject jsonObject;
	QCOMPARE(pfDateTime->toJson(jsonObject), true);
	QCOMPARE(jsonObject.contains("__type"), true);
	QCOMPARE(jsonObject.contains("iso"), true);
	QCOMPARE(jsonObject["__type"].toString(), QString("Date"));
	QCOMPARE(jsonObject["iso"].toString(), QString("2013-11-12T13:16:47.348Z"));
}

void TestPFDateTime::test_pfClassName()
{
	QString parseDateString = "2013-09-15T09:32:00.123Z";
	PFDateTimePtr pfDateTime = PFDateTime::dateTimeFromParseString(parseDateString);
	QCOMPARE(pfDateTime->pfClassName(), QString("PFDateTime"));
}

DECLARE_TEST(TestPFDateTime)
#include "TestPFDateTime.moc"
