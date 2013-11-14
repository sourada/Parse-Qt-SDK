//
//  TestPFDateTime.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#include "PFError.h"
#include "TestRunner.h"

using namespace parse;

class TestPFError : public QObject
{
    Q_OBJECT

private slots:

	// Class init and cleanup methods
	void initTestCase() {}
	void cleanupTestCase() {}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		_normalError = PFError::errorWithCodeAndMessage(kPFErrorUsernameTaken, "Email has already been taken");
		_strangeError = PFError::errorWithCodeAndMessage(0, "");
	}

	void cleanup()
	{
		_normalError = PFErrorPtr();
		_strangeError = PFErrorPtr();
	}

	// Creation Methods
	void test_errorWithCodeAndMessage();

	// Error Accessor Methods
	void test_errorCode();
	void test_errorMessage();

private:

	// Instance members
	PFErrorPtr _normalError;
	PFErrorPtr _strangeError;
};

void TestPFError::test_errorWithCodeAndMessage()
{
	// Normal case
	PFErrorPtr normalError = PFError::errorWithCodeAndMessage(kPFErrorConnectionFailed, "The connection to the Parse servers failed");
	QCOMPARE(normalError->errorCode(), kPFErrorConnectionFailed);
	QCOMPARE(normalError->errorMessage(), QString("The connection to the Parse servers failed"));

	// Strange error but valid
	PFErrorPtr strangeError = PFError::errorWithCodeAndMessage(-20, "");
	QCOMPARE(strangeError->errorCode(), -20);
	QCOMPARE(strangeError->errorMessage(), QString(""));
}

void TestPFError::test_errorCode()
{
	QCOMPARE(_normalError->errorCode(), kPFErrorUsernameTaken);
	QCOMPARE(_strangeError->errorCode(), 0);
}

void TestPFError::test_errorMessage()
{
	QCOMPARE(_normalError->errorMessage(), QString("Email has already been taken"));
	QCOMPARE(_strangeError->errorMessage(), QString(""));
}

DECLARE_TEST(TestPFError)
#include "TestPFError.moc"
