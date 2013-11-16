//
//  TestPFUser.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#include "PFError.h"
#include "PFUser.h"
#include "TestRunner.h"

using namespace parse;

class TestPFUser : public QObject
{
    Q_OBJECT

public slots:

	void signUpCompleted(bool succeeded, PFErrorPtr error)
	{
		_signUpSucceeded = succeeded;
		_signUpError = error;
		emit signUpEnded();
	}

	void logInCompleted(bool succeeded, PFErrorPtr error)
	{
		_logInSucceeded = succeeded;
		_logInError = error;
		emit logInEnded();
	}

	void passwordResetCompleted(bool succeeded, PFErrorPtr error)
	{
		_passwordResetSucceeded = succeeded;
		_passwordResetError = error;
		emit passwordResetEnded();
	}

signals:

	void signUpEnded();
	void logInEnded();
	void passwordResetEnded();

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		// No-op
	}

	void cleanupTestCase()
	{
		// No-op
	}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		// Create a default user
		_defaultUser = PFUser::user();
		_defaultUser->setUsername("parseTestSuiteUser");
		_defaultUser->setEmail("parseTestSuiteUser@parse.com");
		_defaultUser->setPassword("ParseIsAwesome!");

		// Reset all our callback flags
		_signUpSucceeded = false;
		_signUpError = PFErrorPtr();
		_logInSucceeded = false;
		_logInError = PFErrorPtr();
		_passwordResetSucceeded = false;
		_passwordResetError = PFErrorPtr();

		// Make sure we're logged out
		PFUser::logOut();
	}

	void cleanup()
	{
		_defaultUser = PFUserPtr();
	}

	// Creation Methods
	void test_user();
	void test_userWithObjectId();
	void test_currentUser();
	void test_userFromVariant();

	// Authentication Methods
	void test_isAuthenticated();

	// Accessor Methods
	void test_setUsername();
	void test_setEmail();
	void test_setPassword();
	void test_username();
	void test_email();
	void test_password();

	// More Accessor Methods
	void test_parseClassName();
	void test_sessionToken();

	// Sign Up Methods
	void test_signUpWithUser();
	void test_signUpWithUserWithError();
	void test_signUpWithUserInBackground();

	// Log In Methods
	void test_logInWithUsernameAndPassword();
	void test_logInWithUsernameAndPasswordWithError();
	void test_logInWithUsernameAndPasswordInBackground();

	// Password Reset Methods
	void test_requestPasswordResetForEmail();
	void test_requestPasswordResetForEmailWithError();
	void test_requestPasswordResetForEmailInBackground();

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_pfClassName();

	//=================================================================
	//                    Additional Tests
	//=================================================================

	void test_fetchingUsers();

private:

	// Instance members
	PFUserPtr		_defaultUser;
	bool			_signUpSucceeded;
	PFErrorPtr		_signUpError;
	bool			_logInSucceeded;
	PFErrorPtr		_logInError;
	bool			_passwordResetSucceeded;
	PFErrorPtr		_passwordResetError;
};

void TestPFUser::test_user()
{
	PFUserPtr user = PFUser::user();
	QCOMPARE(user.isNull(), false);
	QCOMPARE(user->username(), QString(""));
	QCOMPARE(user->email(), QString(""));
	QCOMPARE(user->password(), QString(""));
	QCOMPARE(user->parseClassName(), QString("_User"));
	QCOMPARE(user->objectId(), QString(""));
	QCOMPARE(user->sessionToken(), QString(""));
	QCOMPARE(user->createdAt(), PFDateTimePtr());
	QCOMPARE(user->updatedAt(), PFDateTimePtr());
}

void TestPFUser::test_userWithObjectId()
{
	// Invalid Case - empty object id
	PFUserPtr invalidUser = PFUser::userWithObjectId("");
	QCOMPARE(invalidUser.isNull(), true);

	// Valid Case
	PFUserPtr validUser = PFUser::userWithObjectId("YsWthslcn0");
	QCOMPARE(validUser.isNull(), false);
	QCOMPARE(validUser->objectId(), QString("YsWthslcn0"));
}

void TestPFUser::test_currentUser()
{
	// Right now we're logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Create a new user just for this test case and retest
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_currentUser");
	testUser->setEmail("test_currentUser@parse.com");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Retest the current user method
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Delete the test user we created
	PFErrorPtr error;
	bool deletedTestUser = testUser->deleteObject(error);
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_userFromVariant()
{
	// Valid Case
	PFUserPtr user = PFUser::user();
	user->setUsername("test_userFromVariant");
	QVariant userVariant = PFObject::toVariant(user);
	PFUserPtr convertedUser = PFUser::userFromVariant(userVariant);
	QCOMPARE(convertedUser.isNull(), false);
	QCOMPARE(convertedUser->username(), QString("test_userFromVariant"));

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFUserPtr convertedString = PFUser::userFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);
}

void TestPFUser::test_isAuthenticated()
{
	// The default user is not authenticated
	QCOMPARE(_defaultUser->isAuthenticated(), false);

	// Sign up a test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_isAuthenticated");
	testUser->setEmail("test_isAuthenticated@parse.com");
	testUser->setPassword("testPassword");
	QCOMPARE(testUser->isAuthenticated(), false);
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(testUser->isAuthenticated(), true);

	// Delete the test user we created
	PFErrorPtr error;
	bool deletedTestUser = testUser->deleteObject(error);
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_setUsername()
{
	QCOMPARE(_defaultUser->username(), QString("parseTestSuiteUser"));
	_defaultUser->setUsername("test_setUsername");
	QCOMPARE(_defaultUser->username(), QString("test_setUsername"));
}

void TestPFUser::test_setEmail()
{
	QCOMPARE(_defaultUser->email(), QString("parseTestSuiteUser@parse.com"));
	_defaultUser->setEmail("test_setUsername@parse.com");
	QCOMPARE(_defaultUser->email(), QString("test_setUsername@parse.com"));
}

void TestPFUser::test_setPassword()
{
	QCOMPARE(_defaultUser->password(), QString("ParseIsAwesome!"));
	_defaultUser->setPassword("testPassword");
	QCOMPARE(_defaultUser->password(), QString("testPassword"));
}

void TestPFUser::test_username()
{
	QCOMPARE(_defaultUser->username(), QString("parseTestSuiteUser"));
	_defaultUser->setUsername("test_setUsername");
	QCOMPARE(_defaultUser->username(), QString("test_setUsername"));
}

void TestPFUser::test_email()
{
	QCOMPARE(_defaultUser->email(), QString("parseTestSuiteUser@parse.com"));
	_defaultUser->setEmail("test_setUsername@parse.com");
	QCOMPARE(_defaultUser->email(), QString("test_setUsername@parse.com"));
}

void TestPFUser::test_password()
{
	QCOMPARE(_defaultUser->password(), QString("ParseIsAwesome!"));
	_defaultUser->setPassword("testPassword");
	QCOMPARE(_defaultUser->password(), QString("testPassword"));
}

void TestPFUser::test_parseClassName()
{
	QCOMPARE(_defaultUser->parseClassName(), QString("_User"));
}

void TestPFUser::test_sessionToken()
{
	QCOMPARE(_defaultUser->sessionToken(), QString(""));

	// Sign up a test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_sessionToken");
	testUser->setEmail("test_sessionToken@parse.com");
	testUser->setPassword("testPassword");
	QCOMPARE(testUser->sessionToken().isEmpty(), true);
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(testUser->sessionToken().isEmpty(), false);

	// Delete the test user we created
	PFErrorPtr error;
	bool deletedTestUser = testUser->deleteObject(error);
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_signUpWithUser()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Sign up a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_signUpWithUser");
	testUser->setEmail("test_signUpWithUser@parse.com");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(testUser->isAuthenticated(), true);
	QCOMPARE(testUser->sessionToken().isEmpty(), false);

	// The new test user should now be the current user
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Delete the test user we created
	bool deletedTestUser = testUser->deleteObject();
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_signUpWithUserWithError()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Sign up a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_signUpWithUserWithError");
	testUser->setEmail("test_signUpWithUserWithError@parse.com");
	testUser->setPassword("testPassword");
	PFErrorPtr error;
	bool signedUp = PFUser::signUpWithUser(testUser, error);
	QCOMPARE(signedUp, true);
	QCOMPARE(error, PFErrorPtr());
	QCOMPARE(testUser->isAuthenticated(), true);
	QCOMPARE(testUser->sessionToken().isEmpty(), false);

	// The new test user should now be the current user
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Delete the test user we created
	bool deletedTestUser = testUser->deleteObject();
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_signUpWithUserInBackground()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Sign up a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_signUpWithUserInBackground");
	testUser->setEmail("test_signUpWithUserInBackground@parse.com");
	testUser->setPassword("testPassword");

	PFUser::signUpWithUserInBackground(testUser, this, SLOT(signUpCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(signUpEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Run some tests to make sure we got all signed up
	QCOMPARE(testUser->isAuthenticated(), true);
	QCOMPARE(testUser->sessionToken().isEmpty(), false);

	// The new test user should now be the current user
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Delete the test user we created
	bool deletedTestUser = testUser->deleteObject();
	QCOMPARE(deletedTestUser, true);
}

void TestPFUser::test_logInWithUsernameAndPassword()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Try to log in with a username/password that definitely does not exist in the cloud
	PFUserPtr invalidUser = PFUser::logInWithUsernameAndPassword("i-am-not-a-valid-user", "this-password-is-bad");
	QCOMPARE(invalidUser, PFUserPtr());
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Let's create a new test user (also tack on some extract properties such as phone number and name)
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_logInWithUsernameAndPassword");
	testUser->setEmail("test_logInWithUsernameAndPassword@parse.com");
	testUser->setPassword("testPassword");
	testUser->setObjectForKey(QString("315-707-8923"), "phoneNumber");
	testUser->setObjectForKey(QString("Bill"), "firstName");
	testUser->setObjectForKey(QString("Lumbergh"), "lastName");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Grab the test user's username and password
	QString username = testUser->username();
	QString password = testUser->password();

	// Destroy the test user and the log out the current user
	testUser = PFUserPtr();
	PFUser::logOut();
	QCOMPARE(testUser, PFUserPtr());
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Try to login with the username and password for the test user which now resides in the cloud
	PFUserPtr validUser = PFUser::logInWithUsernameAndPassword(username, password);
	QCOMPARE(validUser.isNull(), false);
	QCOMPARE(PFUser::currentUser().data(), validUser.data());
	QCOMPARE(validUser->isAuthenticated(), true);

	// Check all the user properties (password should be NULL b/c Parse won't return it)
	QCOMPARE(validUser->username(), QString("test_logInWithUsernameAndPassword"));
	QCOMPARE(validUser->email(), QString("test_logInWithUsernameAndPassword@parse.com"));
	QCOMPARE(validUser->password(), password);
	QCOMPARE(validUser->objectForKey("phoneNumber").toString(), QString("315-707-8923"));
	QCOMPARE(validUser->objectForKey("firstName").toString(), QString("Bill"));
	QCOMPARE(validUser->objectForKey("lastName").toString(), QString("Lumbergh"));

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = validUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_logInWithUsernameAndPasswordWithError()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Try to log in with a username/password that definitely does not exist in the cloud
	PFErrorPtr error;
	PFUserPtr invalidUser = PFUser::logInWithUsernameAndPassword("i-am-not-a-valid-user", "this-password-is-bad", error);
	QCOMPARE(invalidUser, PFUserPtr());
	QCOMPARE(error->errorCode(), kPFErrorObjectNotFound);
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_logInWithUsernameAndPasswordWithError");
	testUser->setEmail("test_logInWithUsernameAndPasswordWithError@parse.com");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Grab the test user's username and password
	QString username = testUser->username();
	QString password = testUser->password();

	// Destroy the test user and the log out the current user
	testUser = PFUserPtr();
	PFUser::logOut();
	QCOMPARE(testUser, PFUserPtr());
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Try to login with the username and password for the test user which now resides in the cloud
	PFErrorPtr error2;
	PFUserPtr validUser = PFUser::logInWithUsernameAndPassword(username, password, error2);
	QCOMPARE(validUser.isNull(), false);
	QCOMPARE(error2, PFErrorPtr());
	QCOMPARE(PFUser::currentUser().data(), validUser.data());
	QCOMPARE(validUser->isAuthenticated(), true);

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = validUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_logInWithUsernameAndPasswordInBackground()
{
	// Before signing up a new user, the current user should be empty since we logged out
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Try to log in with a username/password that definitely does not exist in the cloud
	PFUser::logInWithUsernameAndPasswordInBackground("i-am-not-a-valid-user", "this-password-is-bad", this, SLOT(logInCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(logInEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Test the values that came back from
	QCOMPARE(_logInSucceeded, false);
	QCOMPARE(_logInError.isNull(), false);
	QCOMPARE(_logInError->errorCode(), kPFErrorObjectNotFound);
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Reset the callback values
	_logInSucceeded = false;
	_logInError = PFErrorPtr();

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_logInWithUsernameAndPasswordInBackground");
	testUser->setEmail("test_logInWithUsernameAndPasswordInBackground@parse.com");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);
	QCOMPARE(PFUser::currentUser().data(), testUser.data());

	// Grab the test user's username and password
	QString username = testUser->username();
	QString password = testUser->password();

	// Destroy the test user and the log out the current user
	testUser = PFUserPtr();
	PFUser::logOut();
	QCOMPARE(testUser, PFUserPtr());
	QCOMPARE(PFUser::currentUser(), PFUserPtr());

	// Try to login with the username and password for the test user which now resides in the cloud
	PFUser::logInWithUsernameAndPasswordInBackground(username, password, this, SLOT(logInCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Test the values that came back from
	QCOMPARE(_logInSucceeded, true);
	QCOMPARE(_logInError.isNull(), true);
	QCOMPARE(PFUser::currentUser().isNull(), false);
	PFUserPtr validUser = PFUser::currentUser();
	QCOMPARE(validUser->username(), username);
	QCOMPARE(validUser->password(), password);
	QCOMPARE(validUser->isAuthenticated(), true);

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = validUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_requestPasswordResetForEmail()
{
	// Invalid Case
	bool success = PFUser::requestPasswordResetForEmail("there-is-no-way-this-is-a-valid-email");
	QCOMPARE(success, false);

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_requestPasswordResetForEmail");
	testUser->setEmail("test_requestPasswordResetForEmail@parse.notValid");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Valid Case - try to reset the password for our new user
	success = PFUser::requestPasswordResetForEmail(testUser->email());
	QCOMPARE(success, true);

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = testUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_requestPasswordResetForEmailWithError()
{
	// Invalid Case
	PFErrorPtr error;
	bool success = PFUser::requestPasswordResetForEmail("there-is-no-way-this-is-a-valid-email", error);
	QCOMPARE(success, false);
	QCOMPARE(error.isNull(), false);

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_requestPasswordResetForEmailWithError");
	testUser->setEmail("test_requestPasswordResetForEmailWithError@parse.notValid");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Valid Case - try to reset the password for our new user
	PFErrorPtr error2;
	success = PFUser::requestPasswordResetForEmail(testUser->email(), error2);
	QCOMPARE(success, true);
	QCOMPARE(error2.isNull(), true);

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = testUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_requestPasswordResetForEmailInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Invalid Case
	PFErrorPtr error;
	PFUser::requestPasswordResetForEmailInBackground("there-is-no-way-this-is-a-valid-email", this, SLOT(passwordResetCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(passwordResetEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Test the invalid case results
	QCOMPARE(_passwordResetSucceeded, false);
	QCOMPARE(_passwordResetError.isNull(), false);

	// Reset the test flags
	_passwordResetSucceeded = false;
	_passwordResetError = PFErrorPtr();

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_requestPasswordResetForEmailInBackground");
	testUser->setEmail("test_requestPasswordResetForEmailInBackground@parse.notValid");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Valid Case - try to reset the password for our new user
	PFUser::requestPasswordResetForEmailInBackground(testUser->email(), this, SLOT(passwordResetCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Test the valid case results
	QCOMPARE(_passwordResetSucceeded, true);
	QCOMPARE(_passwordResetError.isNull(), true);

	// Delete our test user (valid user now) from the cloud to cleanup
	bool deletedUser = testUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_fromJson()
{
	// Create a test user
	PFUserPtr user = PFUser::userWithObjectId("09ajf9j3fr49j");

	// Convert the test user to json
	QJsonObject jsonUser;
	user->toJson(jsonUser);

	// Convert the json back to a user
	QVariant userVariant = PFUser::fromJson(jsonUser);
	PFUserPtr convertedUser = PFUser::userFromVariant(userVariant);

	// Test the results of the conversion
	QCOMPARE(convertedUser.isNull(), false);
	QCOMPARE(convertedUser->objectId(), user->objectId());
}

void TestPFUser::test_toJson()
{
	// Invalid Case - try to serialize a file that hasn't been uploaded
	QJsonObject invalidJsonObject;
	QCOMPARE(_defaultUser->toJson(invalidJsonObject), false);

	// Let's create a new test user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_requestPasswordResetForEmailWithError");
	testUser->setEmail("test_requestPasswordResetForEmailWithError@parse.notValid");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Valid Case - serialize the new test user
	QJsonObject jsonObject;
	QCOMPARE(testUser->toJson(jsonObject), true);
	QCOMPARE(jsonObject.contains("__type"), true);
	QCOMPARE(jsonObject.contains("className"), true);
	QCOMPARE(jsonObject.contains("objectId"), true);
	QCOMPARE(jsonObject["__type"].toString(), QString("Pointer"));
	QCOMPARE(jsonObject["className"].toString(), QString("_User"));
	QCOMPARE(jsonObject["objectId"].toString(), testUser->objectId());

	// Delete our test user from the cloud to cleanup
	bool deletedUser = testUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFUser::test_pfClassName()
{
	QCOMPARE(_defaultUser->pfClassName(), QString("PFUser"));
}

void TestPFUser::test_fetchingUsers()
{
	// Sign up a test user with a few properties
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_fetchingUsers");
	testUser->setEmail("test_fetchingUsers@parse.com");
	testUser->setPassword("testPassword");
	testUser->setObjectForKey(QString("315-707-8923"), "phoneNumber");
	testUser->setObjectForKey(QString("Bill"), "firstName");
	testUser->setObjectForKey(QString("Lumbergh"), "lastName");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Create a new user from the objectId and fetch the user with the PFObject fetch method
	PFUserPtr cloudUser = PFUser::userWithObjectId(testUser->objectId());
	QCOMPARE(cloudUser->fetch(), true);

	// Test the fetched properties to ensure we can use the PFObject::fetch methods with PFUsers
	QCOMPARE(cloudUser->username(), QString("test_fetchingUsers"));
	QCOMPARE(cloudUser->email(), QString("test_fetchingUsers@parse.com"));
	QCOMPARE(cloudUser->password().isEmpty(), true);
	QCOMPARE(cloudUser->objectForKey("phoneNumber").toString(), QString("315-707-8923"));
	QCOMPARE(cloudUser->objectForKey("firstName").toString(), QString("Bill"));
	QCOMPARE(cloudUser->objectForKey("lastName").toString(), QString("Lumbergh"));

	// Try to delete the cloud user (should pass since we're still authenticated as user which has the same objectId)
	QCOMPARE(cloudUser->deleteObject(), true);
}

DECLARE_TEST(TestPFUser)
#include "TestPFUser.moc"
