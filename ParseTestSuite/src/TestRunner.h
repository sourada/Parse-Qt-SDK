//
//  TestRunner.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSETESTSUITE_TESTRUNNER_H
#define PARSETESTSUITE_TESTRUNNER_H

// C++ headers
#include <iostream>

// Parse headers
#include "PFManager.h"

// Qt headers
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QtTest>
#include <QTest>

namespace parse {

// Typedefs
typedef QList<QObject*> TestList;

class TestRunner : public QObject
{
	Q_OBJECT

public:

	TestRunner(int argc, char* argv[])
	{
		// Store the args for launching the tests
		_argc = argc;
		_argv = argv;

		// Register the app id and rest api key
		PFManager::sharedManager()->setApplicationIdAndRestApiKey(TestRunner::applicationId(), TestRunner::restApiKey());
		PFManager::sharedManager()->setMasterKey(TestRunner::masterKey());

		// Start up the tests once the application starts
		QTimer::singleShot(100, this, SLOT(runAllTests()));
	}

	// Replace these with your own!!!
	static QString applicationId() { return ""; }
	static QString restApiKey() { return ""; }
	static QString masterKey() { return ""; }

	static TestList& testList()
	{
		static TestList list;
		return list;
	}

	static bool findObject(QObject* object)
	{
		TestList& list = testList();
		if (list.contains(object))
		{
			return true;
		}

		foreach (QObject* test, list)
		{
			if (test->objectName() == object->objectName())
			{
				return true;
			}
		}

		return false;
	}

	static void addTest(QObject* object)
	{
		TestList& list = testList();
		if (!findObject(object))
		{
			list.append(object);
		}
	}

public slots:

	void runAllTests()
	{
		// Make sure the app id and rest api key values are set
		if (applicationId().isEmpty() || restApiKey().isEmpty())
		{
			qCritical() << "The Parse Test Suite can NOT be executed with a blank Application ID and/or Rest API Key...please add them to the TestRunner class";
			QApplication::quit();
		}
		else
		{
			// Run all the tests
			int ret = 0;
			foreach (QObject* test, testList())
			{
				// Run only 1 test
				//			if (test->objectName() == "TestPFObject") // Comment out to run all tests
				{
					ret += QTest::qExec(test, _argc, _argv);
					std::cout << "\n" << std::endl;
				}
			}

			if (ret == 0)
				std::cout << "SUCCESSFULLY COMPLETED ALL TESTS!!!\n" << std::endl;
			else
				std::cout << "FAILED TO COMPLETE ALL TESTS SUCCESSFULLY :-(\n" << std::endl;;

			// Kill the app
			QApplication::quit();
		}
	}

protected:

	// Instance members
	int			_argc;
	char**		_argv;
};

}	// End of parse namespace

template <class T>
class Test
{
public:
	QSharedPointer<T> child;

	Test(const QString& name) : child(new T)
	{
		child->setObjectName(name);
		parse::TestRunner::addTest(child.data());
	}
};

#define DECLARE_TEST(className) static Test<className> t(#className);

#endif	// End of PARSETESTSUITE_TESTRUNNER_H
