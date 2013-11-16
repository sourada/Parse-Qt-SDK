//
//  TestPFACL.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#include "PFACL.h"
#include "PFUser.h"
#include "TestRunner.h"

using namespace parse;

class TestPFACL : public QObject
{
    Q_OBJECT

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		// Create a test user and add it to the cloud to generate an objectId
		_user = PFUser::user();
		_user->setUsername("testPFACL_initTestCase");
		_user->setEmail("testPFACL_initTestCase@parse.com");
		_user->setPassword("testPassword");
		bool signedUp = _user->signUpWithUser(_user);
		QCOMPARE(signedUp, true);
	}

	void cleanupTestCase()
	{
		// Remove the test user
		bool deletedUser = _user->deleteObject();
		QCOMPARE(deletedUser, true);
	}

	// Function init and cleanup methods (called before/after each test)
	void init() {}
	void cleanup() {}

	// Creation Methods
	void test_ACL();
	void test_ACLFromVariant();
	void test_ACLWithUser();
	void test_setDefaultACLWithAccessForCurrentUser();

	// Public Access Methods
	void test_setPublicReadAccess();
	void test_setPublicWriteAccess();
	void test_publicReadAccess();
	void test_publicWriteAccess();

	// Per-User Access Methods
	void test_setReadAccessForUserId();
	void test_setWriteAccessForUserId();
	void test_readAccessForUserId();
	void test_writeAccessForUserId();

	// Explicit Per-User Access Methods
	void test_setReadAccessForUser();
	void test_setWriteAccessForUser();
	void test_readAccessForUser();
	void test_writeAccessForUser();

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_pfClassName();

private:

	// Instance members
	PFUserPtr _user;
};

void TestPFACL::test_ACL()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->publicReadAccess(), false);
	QCOMPARE(acl->publicWriteAccess(), false);
	QCOMPARE(acl->readAccessForUserId(""), false);
	QCOMPARE(acl->writeAccessForUserId(""), false);
}

void TestPFACL::test_ACLFromVariant()
{
	// Valid Case
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	QVariant aclVariant = PFACL::toVariant(acl);
	PFACLPtr convertedACL = PFACL::ACLFromVariant(aclVariant);
	QCOMPARE(convertedACL.isNull(), false);
	QCOMPARE(convertedACL->publicReadAccess(), true);

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFACLPtr convertedString = PFACL::ACLFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);
}

void TestPFACL::test_ACLWithUser()
{
	// Invalid Case 1 - Empty User
	PFACLPtr emptyACL = PFACL::ACLWithUser(PFUserPtr());
	QCOMPARE(emptyACL.isNull(), true);

	// Invalid Case 2 - User with no object id (not in the cloud)
	PFACLPtr noObjectIdACL = PFACL::ACLWithUser(PFUserPtr());
	QCOMPARE(noObjectIdACL.isNull(), true);

	// Valid Case
	PFACLPtr validACL = PFACL::ACLWithUser(_user);
	QCOMPARE(validACL.isNull(), false);
	QCOMPARE(validACL->publicReadAccess(), false);
	QCOMPARE(validACL->publicWriteAccess(), false);
	QCOMPARE(validACL->readAccessForUser(_user), true);
	QCOMPARE(validACL->writeAccessForUser(_user), true);
	QCOMPARE(validACL->readAccessForUserId(_user->objectId()), true);
	QCOMPARE(validACL->writeAccessForUserId(_user->objectId()), true);
}

void TestPFACL::test_setDefaultACLWithAccessForCurrentUser()
{
	// Untestable method - can only call it
	PFACL::setDefaultACLWithAccessForCurrentUser(PFACL::ACL(), true);
	PFACL::setDefaultACLWithAccessForCurrentUser(PFACL::ACL(), false);
}

void TestPFACL::test_setPublicReadAccess()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->publicReadAccess(), false);
	acl->setPublicReadAccess(true);
	QCOMPARE(acl->publicReadAccess(), true);
	acl->setPublicReadAccess(false);
	QCOMPARE(acl->publicReadAccess(), false);
}

void TestPFACL::test_setPublicWriteAccess()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->publicWriteAccess(), false);
	acl->setPublicWriteAccess(true);
	QCOMPARE(acl->publicWriteAccess(), true);
	acl->setPublicWriteAccess(false);
	QCOMPARE(acl->publicWriteAccess(), false);
}

void TestPFACL::test_publicReadAccess()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->publicReadAccess(), false);
	acl->setPublicReadAccess(true);
	QCOMPARE(acl->publicReadAccess(), true);
	acl->setPublicReadAccess(false);
	QCOMPARE(acl->publicReadAccess(), false);
}

void TestPFACL::test_publicWriteAccess()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->publicWriteAccess(), false);
	acl->setPublicWriteAccess(true);
	QCOMPARE(acl->publicWriteAccess(), true);
	acl->setPublicWriteAccess(false);
	QCOMPARE(acl->publicWriteAccess(), false);
}

void TestPFACL::test_setReadAccessForUserId()
{
	PFACLPtr acl = PFACL::ACL();
	QString userObjectId = "93kidkj30jf";
	QCOMPARE(acl->readAccessForUserId(userObjectId), false);
	acl->setReadAccessForUserId(true, userObjectId);
	QCOMPARE(acl->readAccessForUserId(userObjectId), true);
	acl->setReadAccessForUserId(false, userObjectId);
	QCOMPARE(acl->readAccessForUserId(userObjectId), false);
}

void TestPFACL::test_setWriteAccessForUserId()
{
	PFACLPtr acl = PFACL::ACL();
	QString userObjectId = "93kidkj30jf";
	QCOMPARE(acl->writeAccessForUserId(userObjectId), false);
	acl->setWriteAccessForUserId(true, userObjectId);
	QCOMPARE(acl->writeAccessForUserId(userObjectId), true);
	acl->setWriteAccessForUserId(false, userObjectId);
	QCOMPARE(acl->writeAccessForUserId(userObjectId), false);
}

void TestPFACL::test_readAccessForUserId()
{
	PFACLPtr acl = PFACL::ACL();
	QString userObjectId = "93kidkj30jf";
	QCOMPARE(acl->readAccessForUserId(userObjectId), false);
	acl->setReadAccessForUserId(true, userObjectId);
	QCOMPARE(acl->readAccessForUserId(userObjectId), true);
	acl->setReadAccessForUserId(false, userObjectId);
	QCOMPARE(acl->readAccessForUserId(userObjectId), false);
}

void TestPFACL::test_writeAccessForUserId()
{
	PFACLPtr acl = PFACL::ACL();
	QString userObjectId = "93kidkj30jf";
	QCOMPARE(acl->writeAccessForUserId(userObjectId), false);
	acl->setWriteAccessForUserId(true, userObjectId);
	QCOMPARE(acl->writeAccessForUserId(userObjectId), true);
	acl->setWriteAccessForUserId(false, userObjectId);
	QCOMPARE(acl->writeAccessForUserId(userObjectId), false);
}

void TestPFACL::test_setReadAccessForUser()
{
	// Invalid Case 1 - user is empty
	PFACLPtr invalidACL1 = PFACL::ACL();
	QCOMPARE(invalidACL1->setReadAccessForUser(true, PFUserPtr()), false);
	QCOMPARE(invalidACL1->setReadAccessForUser(false, PFUserPtr()), false);

	// Invalid Case 2 - user has no object id
	PFACLPtr invalidACL2 = PFACL::ACL();
	PFUserPtr user = PFUser::user();
	QCOMPARE(invalidACL2->setReadAccessForUser(true, user), false);
	QCOMPARE(invalidACL2->setReadAccessForUser(false, user), false);

	// Valid Case
	PFACLPtr validACL = PFACL::ACL();
	QCOMPARE(validACL->setReadAccessForUser(true, _user), true);
	QCOMPARE(validACL->readAccessForUser(_user), true);
	QCOMPARE(validACL->setReadAccessForUser(false, _user), true);
	QCOMPARE(validACL->readAccessForUser(_user), false);
	QCOMPARE(validACL->setReadAccessForUser(false, _user), true);
}

void TestPFACL::test_setWriteAccessForUser()
{
	// Invalid Case 1 - user is empty
	PFACLPtr invalidACL1 = PFACL::ACL();
	QCOMPARE(invalidACL1->setWriteAccessForUser(true, PFUserPtr()), false);
	QCOMPARE(invalidACL1->setWriteAccessForUser(false, PFUserPtr()), false);

	// Invalid Case 2 - user has no object id
	PFACLPtr invalidACL2 = PFACL::ACL();
	PFUserPtr user = PFUser::user();
	QCOMPARE(invalidACL2->setWriteAccessForUser(true, user), false);
	QCOMPARE(invalidACL2->setWriteAccessForUser(false, user), false);

	// Valid Case
	PFACLPtr validACL = PFACL::ACL();
	QCOMPARE(validACL->setWriteAccessForUser(true, _user), true);
	QCOMPARE(validACL->writeAccessForUser(_user), true);
	QCOMPARE(validACL->setWriteAccessForUser(false, _user), true);
	QCOMPARE(validACL->writeAccessForUser(_user), false);
	QCOMPARE(validACL->setWriteAccessForUser(false, _user), true);
}

void TestPFACL::test_readAccessForUser()
{
	// Invalid Case 1 - user is empty
	PFACLPtr invalidACL1 = PFACL::ACL();
	QCOMPARE(invalidACL1->setReadAccessForUser(true, PFUserPtr()), false);
	QCOMPARE(invalidACL1->setReadAccessForUser(false, PFUserPtr()), false);

	// Invalid Case 2 - user has no object id
	PFACLPtr invalidACL2 = PFACL::ACL();
	PFUserPtr user = PFUser::user();
	QCOMPARE(invalidACL2->setReadAccessForUser(true, user), false);
	QCOMPARE(invalidACL2->setReadAccessForUser(false, user), false);

	// Valid Case
	PFACLPtr validACL = PFACL::ACL();
	QCOMPARE(validACL->setReadAccessForUser(true, _user), true);
	QCOMPARE(validACL->readAccessForUser(_user), true);
	QCOMPARE(validACL->setReadAccessForUser(false, _user), true);
	QCOMPARE(validACL->readAccessForUser(_user), false);
	QCOMPARE(validACL->setReadAccessForUser(false, _user), true);
}

void TestPFACL::test_writeAccessForUser()
{
	// Invalid Case 1 - user is empty
	PFACLPtr invalidACL1 = PFACL::ACL();
	QCOMPARE(invalidACL1->setWriteAccessForUser(true, PFUserPtr()), false);
	QCOMPARE(invalidACL1->setWriteAccessForUser(false, PFUserPtr()), false);

	// Invalid Case 2 - user has no object id
	PFACLPtr invalidACL2 = PFACL::ACL();
	PFUserPtr user = PFUser::user();
	QCOMPARE(invalidACL2->setWriteAccessForUser(true, user), false);
	QCOMPARE(invalidACL2->setWriteAccessForUser(false, user), false);

	// Valid Case
	PFACLPtr validACL = PFACL::ACL();
	QCOMPARE(validACL->setWriteAccessForUser(true, _user), true);
	QCOMPARE(validACL->writeAccessForUser(_user), true);
	QCOMPARE(validACL->setWriteAccessForUser(false, _user), true);
	QCOMPARE(validACL->writeAccessForUser(_user), false);
	QCOMPARE(validACL->setWriteAccessForUser(false, _user), true);
}

void TestPFACL::test_fromJson()
{
	// Create a fake user id
	QString userId = "YsWthslcn0";

	// Create an ACL
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(false);
	acl->setReadAccessForUserId(true, userId);
	acl->setWriteAccessForUserId(true, userId);

	// Convert the ACL to json
	QJsonObject jsonACL;
	QCOMPARE(acl->toJson(jsonACL), true);

	// Convert the json back to an ACL
	QVariant aclVariant = PFACL::fromJson(jsonACL);
	PFACLPtr convertedACL = PFACL::ACLFromVariant(aclVariant);

	// Test out the properties in the converted acl
	QCOMPARE(convertedACL->publicReadAccess(), true);
	QCOMPARE(convertedACL->publicWriteAccess(), false);
	QCOMPARE(convertedACL->readAccessForUserId(userId), true);
	QCOMPARE(convertedACL->writeAccessForUserId(userId), true);
}

void TestPFACL::test_toJson()
{
	// Empty ACL
	PFACLPtr emptyACL = PFACL::ACL();
	QJsonObject emptyJsonObject;
	QCOMPARE(emptyACL->toJson(emptyJsonObject), true);
	QString emptyJsonString = QJsonDocument(emptyJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(emptyJsonString, QString("{}"));

	// Public ACL
	PFACLPtr publicACL = PFACL::ACL();
	publicACL->setPublicReadAccess(true);
	publicACL->setPublicWriteAccess(true);
	QJsonObject publicJsonObject;
	QCOMPARE(publicACL->toJson(publicJsonObject), true);
	QString publicJsonString = QJsonDocument(publicJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(publicJsonString, QString("{\"*\": {\"read\": true,\"write\": true}}"));

	// User ACL
	QString userObjectId = "93kidkj30jf";
	PFACLPtr userACL = PFACL::ACL();
	userACL->setReadAccessForUserId(true, userObjectId);
	userACL->setWriteAccessForUserId(true, userObjectId);
	QJsonObject userJsonObject;
	QCOMPARE(userACL->toJson(userJsonObject), true);
	QString userJsonString = QJsonDocument(userJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(userJsonString, QString("{\"93kidkj30jf\": {\"read\": true,\"write\": true}}"));

	// Multi-user ACL
	QString user2ObjectId = "3p09ajflikj9";
	PFACLPtr multiUserACL = PFACL::ACL();
	multiUserACL->setReadAccessForUserId(true, userObjectId);
	multiUserACL->setWriteAccessForUserId(true, userObjectId);
	multiUserACL->setReadAccessForUserId(true, user2ObjectId);
	multiUserACL->setWriteAccessForUserId(true, user2ObjectId);
	QJsonObject multiUserJsonObject;
	QCOMPARE(multiUserACL->toJson(multiUserJsonObject), true);
	QString multiUserJsonString = QJsonDocument(multiUserJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(multiUserJsonString, QString("{\"3p09ajflikj9\": {\"read\": true,\"write\": true},\"93kidkj30jf\": {\"read\": true,\"write\": true}}"));

	// Mixed ACL
	PFACLPtr mixedACL = PFACL::ACL();
	mixedACL->setPublicReadAccess(true);
	mixedACL->setPublicWriteAccess(false);
	mixedACL->setReadAccessForUserId(true, userObjectId);
	mixedACL->setWriteAccessForUserId(true, userObjectId);
	QJsonObject mixedJsonObject;
	QCOMPARE(mixedACL->toJson(mixedJsonObject), true);
	QString mixedJsonString = QJsonDocument(mixedJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(mixedJsonString, QString("{\"*\": {\"read\": true},\"93kidkj30jf\": {\"read\": true,\"write\": true}}"));

	// Case where reverse the flags to end up with an empty acl
	PFACLPtr removalACL = PFACL::ACL();
	removalACL->setPublicReadAccess(true);
	removalACL->setPublicWriteAccess(true);
	removalACL->setReadAccessForUserId(true, userObjectId);
	removalACL->setWriteAccessForUserId(true, userObjectId);
	removalACL->setPublicReadAccess(false);
	removalACL->setPublicWriteAccess(false);
	removalACL->setReadAccessForUserId(false, userObjectId);
	removalACL->setWriteAccessForUserId(false, userObjectId);
	QJsonObject removalJsonObject;
	QCOMPARE(removalACL->toJson(removalJsonObject), true);
	QString removalJsonString = QJsonDocument(removalJsonObject).toJson(QJsonDocument::Compact);
	QCOMPARE(removalJsonString, QString("{}"));
}

void TestPFACL::test_pfClassName()
{
	PFACLPtr acl = PFACL::ACL();
	QCOMPARE(acl->pfClassName(), QString("PFACL"));
}

DECLARE_TEST(TestPFACL)
#include "TestPFACL.moc"
