//
//  TestPFManager.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/14/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#include "PFManager.h"
#include "TestRunner.h"

using namespace parse;

class TestPFManager : public QObject
{
    Q_OBJECT

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		// Store the properties of the PFManager so we can put them back
		_applicationId = PFManager::sharedManager()->applicationId();
		_restApiKey = PFManager::sharedManager()->restApiKey();
		_cacheDirectory = PFManager::sharedManager()->cacheDirectory();
	}

	void cleanupTestCase()
	{
		// Restore the properties
		PFManager::sharedManager()->setApplicationIdAndRestApiKey(_applicationId, _restApiKey);
		PFManager::sharedManager()->setCacheDirectory(_cacheDirectory);
	}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		// Clear out the app id and rest api key for each test
		PFManager::sharedManager()->setApplicationIdAndRestApiKey("", "");
	}

	void cleanup()
	{
		// No-op
	}

	// Creation Methods
	void test_sharedManager();

	// Getter Setter Methods
	void test_setApplicationIdAndRestApiKey();
	void test_applicationId();
	void test_restApiKey();

	// Caching and Network Methods
	void test_networkAccessManager();
	void test_setCacheDirectory();
	void test_cacheDirectory();
	void test_clearCache();

private:

	// Instance members
	QString		_applicationId;
	QString		_restApiKey;
	QDir		_cacheDirectory;
};

void TestPFManager::test_sharedManager()
{
	// Test the singleton pointer
	PFManager* manager1 = PFManager::sharedManager();
	PFManager* manager2 = PFManager::sharedManager();
	QCOMPARE(manager1, manager2);

	// Test the cache directory
	QDir cacheDirectory = PFManager::sharedManager()->cacheDirectory();
	QCOMPARE(cacheDirectory.absolutePath().isEmpty(), false);
}

void TestPFManager::test_setApplicationIdAndRestApiKey()
{
	// Valid Case 1 - Empty Values
	PFManager::sharedManager()->setApplicationIdAndRestApiKey("", "");
	QCOMPARE(PFManager::sharedManager()->applicationId().isEmpty(), true);
	QCOMPARE(PFManager::sharedManager()->restApiKey().isEmpty(), true);

	// Valid Case 2 - Proper Values
	PFManager::sharedManager()->setApplicationIdAndRestApiKey("My Made Up App Id", "My Made Up Rest API Key");
	QCOMPARE(PFManager::sharedManager()->applicationId().isEmpty(), false);
	QCOMPARE(PFManager::sharedManager()->restApiKey().isEmpty(), false);
}

void TestPFManager::test_applicationId()
{
	// Should be empty now
	QCOMPARE(PFManager::sharedManager()->applicationId().isEmpty(), true);

	// Modify and retest
	PFManager::sharedManager()->setApplicationIdAndRestApiKey("My Made Up App Id", "My Made Up Rest API Key");
	QCOMPARE(PFManager::sharedManager()->applicationId().isEmpty(), false);
}

void TestPFManager::test_restApiKey()
{
	// Should be empty now
	QCOMPARE(PFManager::sharedManager()->restApiKey().isEmpty(), true);

	// Modify and retest
	PFManager::sharedManager()->setApplicationIdAndRestApiKey("My Made Up App Id", "My Made Up Rest API Key");
	QCOMPARE(PFManager::sharedManager()->restApiKey().isEmpty(), false);
}

void TestPFManager::test_networkAccessManager()
{
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QCOMPARE(networkAccessManager == NULL, false);
	QCOMPARE(networkAccessManager->networkAccessible(), QNetworkAccessManager::Accessible);
}

void TestPFManager::test_setCacheDirectory()
{
	QDir tempDirectory = QDir::temp();
	PFManager::sharedManager()->setCacheDirectory(tempDirectory);
	QCOMPARE(PFManager::sharedManager()->cacheDirectory().absolutePath(), tempDirectory.absolutePath());
}

void TestPFManager::test_cacheDirectory()
{
	QDir tempDirectory = QDir::temp();
	PFManager::sharedManager()->setCacheDirectory(tempDirectory);
	QCOMPARE(PFManager::sharedManager()->cacheDirectory().absolutePath(), tempDirectory.absolutePath());
}

void TestPFManager::test_clearCache()
{
	// Create a new folder in the temp directory and set it as the cache directory
	QString folder = "TestPFManager-test_clearCache";
	QDir tempDirectory = QDir::temp();
	tempDirectory.mkdir(folder);
	QCOMPARE(tempDirectory.cd(folder), true);
	PFManager::sharedManager()->setCacheDirectory(tempDirectory);
	QCOMPARE(PFManager::sharedManager()->cacheDirectory().absolutePath(), tempDirectory.absolutePath());

	// Add some stuff to that folder
	int currentCount = PFManager::sharedManager()->cacheDirectory().count();
	PFManager::sharedManager()->cacheDirectory().mkdir("Dummy Folder 1");
	PFManager::sharedManager()->cacheDirectory().mkdir("Dummy Folder 2");
	PFManager::sharedManager()->cacheDirectory().mkdir("Dummy Folder 3");
	PFManager::sharedManager()->cacheDirectory().refresh();
	QCOMPARE(currentCount + 3, (int) PFManager::sharedManager()->cacheDirectory().count());

	// Clear the cache
	PFManager::sharedManager()->clearCache();
	PFManager::sharedManager()->cacheDirectory().refresh();
	QCOMPARE(currentCount,  (int) PFManager::sharedManager()->cacheDirectory().count());
	QCOMPARE(tempDirectory.absolutePath(), PFManager::sharedManager()->cacheDirectory().absolutePath());

	// Remove the test cache directory
	QCOMPARE(PFManager::sharedManager()->cacheDirectory().removeRecursively(), true);
}

DECLARE_TEST(TestPFManager)
#include "TestPFManager.moc"
