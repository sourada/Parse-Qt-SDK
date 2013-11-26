//
//  TestPFObject.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "PFACL.h"
#include "PFDateTime.h"
#include "PFError.h"
#include "PFFile.h"
#include "PFObject.h"
#include "PFUser.h"
#include "TestRunner.h"

using namespace parse;

class TestPFObject : public QObject
{
    Q_OBJECT

public slots:

	void saveCompleted(bool succeeded, PFErrorPtr error)
	{
		_saveSucceeded = succeeded;
		_saveError = error;
		emit saveEnded();
	}

	void deleteObjectCompleted(bool succeeded, PFErrorPtr error)
	{
		_deleteObjectSucceeded = succeeded;
		_deleteObjectError = error;
		emit deleteObjectEnded();
	}

	void fetchCompleted(bool succeeded, PFErrorPtr error)
	{
		_fetchSucceeded = succeeded;
		_fetchError = error;
		emit fetchEnded();
	}

signals:

	void saveEnded();
	void deleteObjectEnded();
	void fetchEnded();

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		// Make sure the default ACL is reset
		PFACL::setDefaultACLWithAccessForCurrentUser(PFACLPtr(), false);

		// Make sure we're logged out
		PFUser::logOut();

		// Build some default data for a file
		_data = QByteArrayPtr(new QByteArray());
		_data->append(QString("Some sample data to test with").toUtf8());

		// Set the data path
		QDir currentDir = QDir::current();
		currentDir.cdUp();
		_dataPath = currentDir.absoluteFilePath("data");

		// Build the object graph for testing
		buildObjectGraph();
	}

	void cleanupTestCase()
	{
		deleteObjectGraph();
		this->disconnect();
	}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		// Reset the default ACL
		PFACL::setDefaultACLWithAccessForCurrentUser(PFACLPtr(), false);

		// Reset the callback flags
		_saveSucceeded = false;
		_saveError = PFErrorPtr();
		_deleteObjectSucceeded = false;
		_deleteObjectError = PFErrorPtr();
		_fetchSucceeded = false;
		_fetchError = PFErrorPtr();

		// Make sure we're completely disconnect from anything
		this->disconnect();
	}

	void cleanup()
	{
		// No-op
	}

	// Creation Methods
	void test_objectWithClassName();
	void test_objectWithClassNameAndObjectId();
	void test_objectFromVariant();

	// Object Storage Methods
	void test_setObjectForKey();
	void test_setObjectForKeyWithSerializable();
	void test_removeObjectForKey();
	void test_objectForKey();
	void test_allKeys();

	// Increment Methods
	void test_incrementKey();
	void test_incrementKeyByAmount();

	// List Add & Remove Methods
	void test_addObjectToListForKey();
	void test_addObjectToListForKeyWithSerializable();
	void test_addObjectsToListForKey();
	void test_addUniqueObjectToListForKey();
	void test_addUniqueObjectToListForKeyWithSerializable();
	void test_addUniqueObjectsToListForKey();
	void test_removeObjectFromListForKey();
	void test_removeObjectFromListForKeyWithSerializable();
	void test_removeObjectsFromListForKey();

	// ACL Accessor Methods
	void test_setACL();
	void test_ACL();

	// Object Info Getter Methods
	void test_parseClassName();
	void test_objectId();
	void test_createdAt();
	void test_updatedAt();

	// Save Methods
	void test_save();
	void test_saveWithError();
	void test_saveInBackground();

	// Delete Methods
	void test_deleteObject();
	void test_deleteObjectWithError();
	void test_deleteObjectInBackground();

	// Data Availability Methods
	void test_isDataAvailable();

	// Fetch Methods
	void test_fetch();
	void test_fetchWithError();
	void test_fetchInBackground();

	// Fetch If Needed Methods
	void test_fetchIfNeeded();
	void test_fetchIfNeededWithError();
	void test_fetchIfNeededInBackground();

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_pfClassName();

	//=================================================================
	//                    Additional Tests
	//=================================================================

	void test_savingWithPermissions();
	void test_fetchingWithPermissions();

private:

	void buildObjectGraph()
	{
		/**
		 * Objects
		 *
		 *   - PFUser (testUser)
		 *   - PFDateTime (expirationDate)
		 *   - PFObject (Character) - sidekick, hero
		 *   - PFObject (Armor) - greaves, helmet
		 *   - PFObject (Weapon) - axe, sword
		 *   - PFFile (tutorialFile)
		 *   - PFACL (acl)
		 *
		 * Object Graph
		 *
		 *   - hero
		 *       | - name (QString)
		 *       | - characterType (QString)
		 *       | - power (int)
		 *       | - sidekick (PFObject)
		 *       | - ACL (PFACL)
		 *       | - tutorialFile (PFFile)
		 *       | - weapons (array)
		 *       |       | - axe (PFObject)
		 *       |       | - sword (PFObject)
		 *       |
		 *       | - armor (map)
		 *               | - greaves (PFObject)
		 *               | - helmet (PFObject)
		 *
		 *   - sidekick
		 *       | - name (QString)
		 *       | - characterType (QString)
		 *       | - power (int)
		 *
		 *   - axe
		 *       | - name (QString)
		 *       | - attackMin (int)
		 *       | - attackMax (int)
		 *       | - strengthRequired (int)
		 *
		 *   - sword
		 *       | - name (QString)
		 *       | - attackMin (int)
		 *       | - attackMax (int)
		 *       | - strengthRequired (int)
		 *
		 *   - greaves
		 *       | - name (QString)
		 *       | - defense (int)
		 *
		 *   - helmet
		 *       | - name (QString)
		 *       | - defense (int)
		 *
		 */

		// PFUser (testUser)
		_testUser = PFUser::user();
		_testUser->setUsername("TestPFObject");
		_testUser->setEmail("TestPFObject@parse.com");
		_testUser->setPassword("testPassword");
		bool signedUp = PFUser::signUpWithUser(_testUser);
		QCOMPARE(signedUp, true);

		// PFObject (greaves) - Armor
		_greaves = PFObject::objectWithClassName("Armor");
		_greaves->setObjectForKey(QString("Greaves"), "name");
		_greaves->setObjectForKey(26, "defense");
		bool greavesSaved = _greaves->save();
		QCOMPARE(greavesSaved, true);

		// PFObject (helmet) - Armor
		_helmet = PFObject::objectWithClassName("Armor");
		_helmet->setObjectForKey(QString("Helmet"), "name");
		_helmet->setObjectForKey(121, "defense");
		bool helmetSaved = _helmet->save();
		QCOMPARE(helmetSaved, true);

		// PFObject (axe) - Weapon
		_axe = PFObject::objectWithClassName("Weapon");
		_axe->setObjectForKey(QString("Axe"), "weaponClass");
		_axe->setObjectForKey(19, "attackMin");
		_axe->setObjectForKey(26, "attackMax");
		_axe->setObjectForKey(65, "strengthRequired");
		bool axeSaved = _axe->save();
		QCOMPARE(axeSaved, true);

		// PFObject (sword) - Weapon
		_sword = PFObject::objectWithClassName("Weapon");
		_sword->setObjectForKey(QString("Sword"), "weaponClass");
		_sword->setObjectForKey(13, "attackMin");
		_sword->setObjectForKey(15, "attackMax");
		_sword->setObjectForKey(29, "strengthRequired");
		bool swordSaved = _sword->save();
		QCOMPARE(swordSaved, true);

		// PFFile (tutorialFile)
		QByteArrayPtr data = QByteArrayPtr(new QByteArray(QString("The Diablo Tutorial Guide, Part I").toUtf8()));
		_tutorialFile = PFFile::fileWithData(data);
		bool tutorialFileSaved = _tutorialFile->save();
		QCOMPARE(tutorialFileSaved, true);

		// PFACL (acl)
		PFACLPtr acl = PFACL::ACL();
		acl->setPublicReadAccess(true);
		acl->setPublicWriteAccess(true);

		// PFDateTime (expirationDate)
		_expirationDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());

		// PFObject (sidekick) - Character
		_sidekick = PFObject::objectWithClassName("Character");
		_sidekick->setObjectForKey(QString("Menalous"), "name");
		_sidekick->setObjectForKey(QString("Sidekick"), "characterType");
		_sidekick->setObjectForKey(24, "power");
		bool sidekickSaved = _sidekick->save();
		QCOMPARE(sidekickSaved, true);

		// Weapons List
		QVariantList weapons;
		weapons << PFObject::toVariant(_axe) << PFObject::toVariant(_sword);

		// Armor Map
		QVariantMap armor;
		armor["helmet"] = PFObject::toVariant(_helmet);
		armor["greaves"] = PFObject::toVariant(_greaves);

		// PFObject (hero) - Character
		_hero = PFObject::objectWithClassName("Character");
		_hero->setObjectForKey(QString("Hercules"), "name");
		_hero->setObjectForKey(QString("Hero"), "characterType");
		_hero->setObjectForKey(80, "power");
		_hero->setACL(acl);
		_hero->setObjectForKey(_expirationDate, "expirationDate");
		_hero->setObjectForKey(_tutorialFile, "tutorialFile");
		_hero->setObjectForKey(_sidekick, "sidekick");
		_hero->setObjectForKey(_testUser, "accountOwner");
		_hero->setObjectForKey(weapons, "weapons");
		_hero->setObjectForKey(armor, "armor");
		bool heroSaved = _hero->save();
		QCOMPARE(heroSaved, true);
	}

	void deleteObjectGraph()
	{
		// Re-authenticate testUser if necessary
		if (!_testUser->isAuthenticated())
		{
			QCOMPARE(PFUser::logInWithUsernameAndPassword(_testUser->username(), _testUser->password()).isNull(), false);
			_testUser = PFUser::currentUser();
		}

		// Cleanup
		QCOMPARE(_testUser->deleteObject(), true);
		QCOMPARE(_greaves->deleteObject(), true);
		QCOMPARE(_helmet->deleteObject(), true);
		QCOMPARE(_axe->deleteObject(), true);
		QCOMPARE(_sword->deleteObject(), true);
		QCOMPARE(_sidekick->deleteObject(), true);
		QCOMPARE(_hero->deleteObject(), true);
	}

	// Instance members
	QString			_dataPath;
	QByteArrayPtr	_data;
	bool			_saveSucceeded;
	PFErrorPtr		_saveError;
	bool			_deleteObjectSucceeded;
	PFErrorPtr		_deleteObjectError;
	bool			_fetchSucceeded;
	PFErrorPtr		_fetchError;

	// Instance Members - Object Graph
	PFUserPtr		_testUser;
	PFObjectPtr		_greaves;
	PFObjectPtr		_helmet;
	PFObjectPtr		_axe;
	PFObjectPtr		_sword;
	PFObjectPtr		_sidekick;
	PFObjectPtr		_hero;
	PFDateTimePtr	_expirationDate;
	PFFilePtr		_tutorialFile;
};

void TestPFObject::test_objectWithClassName()
{
	// Invalid Object - Empty Class Name
	PFObjectPtr invalidObject = PFObject::objectWithClassName("");
	QCOMPARE(invalidObject.isNull(), true);

	// Valid Object - Level
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level.isNull(), false);
	QCOMPARE(level->className(), QString("Level"));
	QCOMPARE(level->objectId(), QString(""));
	QCOMPARE(level->ACL().isNull(), true);
	QCOMPARE(level->createdAt(), PFDateTimePtr());
	QCOMPARE(level->updatedAt(), PFDateTimePtr());

	// Set the default ACL - no current user
	PFACLPtr defaultACL = PFACL::ACL();
	defaultACL->setPublicReadAccess(true);
	PFACL::setDefaultACLWithAccessForCurrentUser(defaultACL, false);

	// Create another object and test the ACL
	PFObjectPtr level2 = PFObject::objectWithClassName("Level");
	QCOMPARE(level2.isNull(), false);
	QCOMPARE(level2->ACL().isNull(), false);
	QCOMPARE(level2->ACL()->publicReadAccess(), true);
	QCOMPARE(level2->ACL()->publicWriteAccess(), false);

	// Change the default ACL to use the current user - but no current user is set during creation
	PFACLPtr userACL = PFACL::ACL();
	userACL->setPublicWriteAccess(true);
	PFACL::setDefaultACLWithAccessForCurrentUser(userACL, true);

	// Create another object and test the ACL
	PFObjectPtr level3 = PFObject::objectWithClassName("Level");
	QCOMPARE(level3.isNull(), false);
	QCOMPARE(level3->ACL().isNull(), false);
	QCOMPARE(level3->ACL()->publicReadAccess(), false);
	QCOMPARE(level3->ACL()->publicWriteAccess(), true);

	// Create a current user
	PFUserPtr testUser = PFUser::user();
	testUser->setUsername("test_objectWithClassName");
	testUser->setEmail("test_objectWithClassName@parse.com");
	testUser->setPassword("testPassword");
	bool signedUp = PFUser::signUpWithUser(testUser);
	QCOMPARE(signedUp, true);

	// Create a final object and test the ACL to make sure it has permissions for the current user
	PFObjectPtr level4 = PFObject::objectWithClassName("Level");
	QCOMPARE(level4.isNull(), false);
	QCOMPARE(level4->ACL().isNull(), false);
	QCOMPARE(level4->ACL()->publicReadAccess(), false);
	QCOMPARE(level4->ACL()->publicWriteAccess(), true);
	QCOMPARE(level4->ACL()->readAccessForUser(PFUser::currentUser()), true);
	QCOMPARE(level4->ACL()->writeAccessForUser(PFUser::currentUser()), true);

	// Delete our test user from the cloud to cleanup
	bool deletedUser = testUser->deleteObject();
	QCOMPARE(deletedUser, true);
}

void TestPFObject::test_objectWithClassNameAndObjectId()
{
	// Invalid Object Case 1 - Empty Class Name
	PFObjectPtr invalidObject1 = PFObject::objectWithClassName("", "309di3j09f");
	QCOMPARE(invalidObject1.isNull(), true);

	// Invalid Object Case 2 - Empty Object ID
	PFObjectPtr invalidObject2 = PFObject::objectWithClassName("Level", "");
	QCOMPARE(invalidObject2.isNull(), true);

	// Invalid Object Case 3 - Empty Class Name and Object ID
	PFObjectPtr invalidObject3 = PFObject::objectWithClassName("", "");
	QCOMPARE(invalidObject3.isNull(), true);

	// Valid Object
	PFObjectPtr level = PFObject::objectWithClassName("Level", "309di3j09f");
	QCOMPARE(level.isNull(), false);
	QCOMPARE(level->className(), QString("Level"));
	QCOMPARE(level->objectId(), QString("309di3j09f"));
	QCOMPARE(level->ACL().isNull(), true);
	QCOMPARE(level->createdAt(), PFDateTimePtr());
	QCOMPARE(level->updatedAt(), PFDateTimePtr());
}

void TestPFObject::test_objectFromVariant()
{
	// Valid Case
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QVariant levelVariant = PFObject::toVariant(level);
	PFObjectPtr convertedLevel = PFObject::objectFromVariant(levelVariant);
	QCOMPARE(convertedLevel.isNull(), false);
	QCOMPARE(convertedLevel->className(), QString("Level"));

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFObjectPtr convertedString = PFObject::objectFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);

	// Invalid Case - PFDateTime
	PFDateTimePtr dateTime = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	QVariant dateTimeVariant = PFDateTime::toVariant(dateTime);
	PFObjectPtr dateTimeObject = PFObject::objectFromVariant(dateTimeVariant);
	QCOMPARE(dateTimeObject.isNull(), true);

	// Invalid Case - PFFile
	PFFilePtr file = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	QVariant fileVariant = PFFile::toVariant(file);
	PFObjectPtr fileObject = PFObject::objectFromVariant(fileVariant);
	QCOMPARE(fileObject.isNull(), true);
}

void TestPFObject::test_setObjectForKey()
{
	// Create test object
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level.isNull(), false);

	// QString objects
	level->setObjectForKey(QString("Act I"), "name");
	QCOMPARE(level->objectForKey("name").toString(), QString("Act I"));

	// Number objects
	level->setObjectForKey(1234, "points");
	level->setObjectForKey(0.5489, "normalizedValue");
	QCOMPARE(level->objectForKey("points").toInt(), (int) 1234);
	QCOMPARE(level->objectForKey("normalizedValue").toFloat(), (float) 0.5489);

	// Bool objects
	level->setObjectForKey(false, "cold");
	level->setObjectForKey(false, "fire");
	level->setObjectForKey(true, "poison");
	QCOMPARE(level->objectForKey("cold").toBool(), false);
	QCOMPARE(level->objectForKey("fire").toBool(), false);
	QCOMPARE(level->objectForKey("poison").toBool(), true);

	// List objects
	QList<QVariant> characters;
	characters.append(QString("Maverick"));
	characters.append(QString("Goose"));
	characters.append(QString("IceMan"));
	level->setObjectForKey(QVariant(characters), "characters");
	QList<QVariant> levelCharacters = level->objectForKey("characters").toList();
	QCOMPARE(levelCharacters.at(0).toString(), QString("Maverick"));
	QCOMPARE(levelCharacters.at(1).toString(), QString("Goose"));
	QCOMPARE(levelCharacters.at(2).toString(), QString("IceMan"));

	// Map Objects
	QMap<QString, QVariant> weaponDamage;
	weaponDamage["sword"] = (int) 4;
	weaponDamage["axe"] = (int) 5;
	weaponDamage["wand"] = (int) 1;
	level->setObjectForKey(weaponDamage, "weaponDamage");
	QMap<QString, QVariant> levelWeaponDamage = level->objectForKey("weaponDamage").toMap();
	QCOMPARE(levelWeaponDamage.contains("sword"), true);
	QCOMPARE(levelWeaponDamage.contains("axe"), true);
	QCOMPARE(levelWeaponDamage.contains("wand"), true);
	QCOMPARE(levelWeaponDamage.contains("club"), false);
	QCOMPARE(levelWeaponDamage["sword"].toInt(), (int) 4);
	QCOMPARE(levelWeaponDamage["axe"].toInt(), (int) 5);
	QCOMPARE(levelWeaponDamage["wand"].toInt(), (int) 1);
}

void TestPFObject::test_setObjectForKeyWithSerializable()
{
	// Create test object
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level.isNull(), false);

	// PFObject
	PFObjectPtr character = PFObject::objectWithClassName("Character");
	QCOMPARE(character.isNull(), false);
	character->setObjectForKey(QString("Maverick"), "name");
	level->setObjectForKey(character, "character");
	PFObjectPtr levelCharacter = PFObject::objectFromVariant(level->objectForKey("character"));
	QCOMPARE(levelCharacter.isNull(), false);
	QCOMPARE(levelCharacter->objectForKey("name").toString(), QString("Maverick"));

	// PFDateTime
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	QCOMPARE(expirationDate.isNull(), false);
	level->setObjectForKey(expirationDate, "expirationDate");
	PFDateTimePtr levelExpirationDate = PFDateTime::dateTimeFromVariant(level->objectForKey("expirationDate"));
	QCOMPARE(levelExpirationDate.isNull(), false);
	QCOMPARE(levelExpirationDate->dateTime().date(), QDate::currentDate());

	// PFFile
	PFFilePtr tutorialFile = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	QCOMPARE(tutorialFile.isNull(), false);
	level->setObjectForKey(tutorialFile, "tutorialFile");
	PFFilePtr levelTutorialFile = PFFile::fileFromVariant(level->objectForKey("tutorialFile"));
	QCOMPARE(tutorialFile.isNull(), false);
	QCOMPARE(tutorialFile->name(), QString("tutorialFile.txt"));

	// List of PFObjects
	PFObjectPtr town1 = PFObject::objectWithClassName("Town");
	town1->setObjectForKey(QString("Tristam"), "name");
	PFObjectPtr town2 = PFObject::objectWithClassName("Town");
	town2->setObjectForKey(QString("Burial Grounds"), "name");
	QVariantList towns;
	towns << PFObject::toVariant(town1) << PFObject::toVariant(town2);
	level->setObjectForKey(towns, "towns");
	QVariantList levelTowns = level->objectForKey("towns").toList();
	QCOMPARE(levelTowns.count(), 2);
	PFObjectPtr levelTown1 = PFObject::objectFromVariant(levelTowns.at(0));
	QCOMPARE(levelTown1.isNull(), false);
	QCOMPARE(levelTown1->objectForKey("name").toString(), QString("Tristam"));
	PFObjectPtr levelTown2 = PFObject::objectFromVariant(levelTowns.at(1));
	QCOMPARE(levelTown2.isNull(), false);
	QCOMPARE(levelTown2->objectForKey("name").toString(), QString("Burial Grounds"));
}

void TestPFObject::test_removeObjectForKey()
{
	// Create an object
	PFObjectPtr lamp = PFObject::objectWithClassName("Furniture");

	// Try to remove a key that doesn't exist
	QCOMPARE(lamp->removeObjectForKey("ThisKeyDoesNotExist"), false);

	// Let's add a key then remove it
	lamp->setObjectForKey(QString("Lamp"), "furnitureType");
	QCOMPARE(lamp->removeObjectForKey("furnitureType"), true);
	QCOMPARE(lamp->allKeys().isEmpty(), true);

	// Add the key again and save the object
	lamp->setObjectForKey(QString("Lamp"), "furnitureType");
	QCOMPARE(lamp->save(), true);

	// Fetch the object from the server and verify the key was saved
	PFObjectPtr cloudLamp = PFObject::objectWithClassName(lamp->className(), lamp->objectId());
	QCOMPARE(cloudLamp->fetch(), true);
	QCOMPARE(cloudLamp->objectForKey("furnitureType").toString(), QString("Lamp"));

	// Remove the key from the lamp
	QCOMPARE(lamp->removeObjectForKey("furnitureType"), true);

	// Save the lamp again, then re-fetch the cloud lamp and verify the key is gone
	QCOMPARE(lamp->save(), true);
	QCOMPARE(cloudLamp->fetch(), true);
	QCOMPARE(cloudLamp->objectForKey("furnitureType").isValid(), false);
	QCOMPARE(cloudLamp->allKeys().isEmpty(), true);

	// Cleanup
	QCOMPARE(lamp->deleteObject(), true);
}

void TestPFObject::test_objectForKey()
{
	/////////////////////////////////////////////////////////////////////////////////
	//                             Object Creation
	/////////////////////////////////////////////////////////////////////////////////

	// Create test object
	PFObjectPtr level = PFObject::objectWithClassName("Level");

	// QString objects
	level->setObjectForKey(QString("Act I"), "name");

	// Number objects
	level->setObjectForKey(1234, "points");
	level->setObjectForKey(0.5489, "normalizedValue");

	// Bool objects
	level->setObjectForKey(false, "cold");
	level->setObjectForKey(false, "fire");
	level->setObjectForKey(true, "poison");

	// List objects
	QList<QVariant> characters;
	characters.append(QString("Maverick"));
	characters.append(QString("Goose"));
	characters.append(QString("IceMan"));
	level->setObjectForKey(QVariant(characters), "characters");

	// Map Objects
	QMap<QString, QVariant> weaponDamage;
	weaponDamage["sword"] = (int) 4;
	weaponDamage["axe"] = (int) 5;
	weaponDamage["wand"] = (int) 1;
	level->setObjectForKey(weaponDamage, "weaponDamage");

	// PFObject
	PFObjectPtr character = PFObject::objectWithClassName("Character");
	character->setObjectForKey(QString("Maverick"), "name");
	level->setObjectForKey(character, "character");

	// PFDateTime
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(expirationDate, "expirationDate");

	// PFFile
	PFFilePtr tutorialFile = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	level->setObjectForKey(tutorialFile, "tutorialFile");

	/////////////////////////////////////////////////////////////////////////////////
	//                                 Tests
	/////////////////////////////////////////////////////////////////////////////////

	// QString objects
	QCOMPARE(level->objectForKey("name").toString(), QString("Act I"));

	// Number objects
	QCOMPARE(level->objectForKey("points").toInt(), (int) 1234);
	QCOMPARE(level->objectForKey("normalizedValue").toFloat(), (float) 0.5489);

	// Bool objects
	QCOMPARE(level->objectForKey("cold").toBool(), false);
	QCOMPARE(level->objectForKey("fire").toBool(), false);
	QCOMPARE(level->objectForKey("poison").toBool(), true);

	// List objects
	QList<QVariant> levelCharacters = level->objectForKey("characters").toList();
	QCOMPARE(levelCharacters.at(0).toString(), QString("Maverick"));
	QCOMPARE(levelCharacters.at(1).toString(), QString("Goose"));
	QCOMPARE(levelCharacters.at(2).toString(), QString("IceMan"));

	// Map Objects
	QMap<QString, QVariant> levelWeaponDamage = level->objectForKey("weaponDamage").toMap();
	QCOMPARE(levelWeaponDamage.contains("sword"), true);
	QCOMPARE(levelWeaponDamage.contains("axe"), true);
	QCOMPARE(levelWeaponDamage.contains("wand"), true);
	QCOMPARE(levelWeaponDamage.contains("club"), false);
	QCOMPARE(levelWeaponDamage["sword"].toInt(), (int) 4);
	QCOMPARE(levelWeaponDamage["axe"].toInt(), (int) 5);
	QCOMPARE(levelWeaponDamage["wand"].toInt(), (int) 1);

	// PFObject
	PFObjectPtr levelCharacter = PFObject::objectFromVariant(level->objectForKey("character"));
	QCOMPARE(levelCharacter.isNull(), false);
	QCOMPARE(levelCharacter->objectForKey("name").toString(), QString("Maverick"));

	// PFDateTime
	PFDateTimePtr levelExpirationDate = PFDateTime::dateTimeFromVariant(level->objectForKey("expirationDate"));
	QCOMPARE(levelExpirationDate.isNull(), false);
	QCOMPARE(levelExpirationDate->dateTime().date(), QDate::currentDate());

	// PFFile
	PFFilePtr levelTutorialFile = PFFile::fileFromVariant(level->objectForKey("tutorialFile"));
	QCOMPARE(tutorialFile.isNull(), false);
	QCOMPARE(tutorialFile->name(), QString("tutorialFile.txt"));
}

void TestPFObject::test_allKeys()
{
	/////////////////////////////////////////////////////////////////////////////////
	//                             Object Creation
	/////////////////////////////////////////////////////////////////////////////////

	// Create test object
	PFObjectPtr level = PFObject::objectWithClassName("Level");

	// QString objects
	level->setObjectForKey(QString("Act I"), "name");

	// Number objects
	level->setObjectForKey(1234, "points");
	level->setObjectForKey(0.5489, "normalizedValue");

	// Bool objects
	level->setObjectForKey(false, "cold");
	level->setObjectForKey(false, "fire");
	level->setObjectForKey(true, "poison");

	// List objects
	QList<QVariant> characters;
	characters.append(QString("Maverick"));
	characters.append(QString("Goose"));
	characters.append(QString("IceMan"));
	level->setObjectForKey(QVariant(characters), "characters");

	// Map Objects
	QMap<QString, QVariant> weaponDamage;
	weaponDamage["sword"] = (int) 4;
	weaponDamage["axe"] = (int) 5;
	weaponDamage["wand"] = (int) 1;
	level->setObjectForKey(weaponDamage, "weaponDamage");

	// PFObject
	PFObjectPtr character = PFObject::objectWithClassName("Character");
	character->setObjectForKey(QString("Maverick"), "name");
	level->setObjectForKey(character, "character");

	// PFDateTime
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(expirationDate, "expirationDate");

	// PFFile
	PFFilePtr tutorialFile = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	level->setObjectForKey(tutorialFile, "tutorialFile");

	/////////////////////////////////////////////////////////////////////////////////
	//                                 Tests
	/////////////////////////////////////////////////////////////////////////////////

	// Test keys actually in the level
	QStringList slowKeys = level->allKeys();
	QSet<QString> fastKeys = slowKeys.toSet();
	QCOMPARE(fastKeys.count(), 11);
	QCOMPARE(fastKeys.contains("name"), true);
	QCOMPARE(fastKeys.contains("points"), true);
	QCOMPARE(fastKeys.contains("normalizedValue"), true);
	QCOMPARE(fastKeys.contains("cold"), true);
	QCOMPARE(fastKeys.contains("fire"), true);
	QCOMPARE(fastKeys.contains("poison"), true);
	QCOMPARE(fastKeys.contains("characters"), true);
	QCOMPARE(fastKeys.contains("weaponDamage"), true);
	QCOMPARE(fastKeys.contains("character"), true);
	QCOMPARE(fastKeys.contains("expirationDate"), true);
	QCOMPARE(fastKeys.contains("tutorialFile"), true);

	// Test keys that aren't in the level
	QCOMPARE(fastKeys.contains("injuries"), false);
	QCOMPARE(fastKeys.contains("deadCount"), false);
	QCOMPARE(fastKeys.contains("expirations"), false);

	// Create an empty object and test
	PFObjectPtr level2 = PFObject::objectWithClassName("Level");
	QStringList level2Keys = level2->allKeys();
	QCOMPARE(level2Keys.count(), 0);
}

void TestPFObject::test_incrementKey()
{
	// Invalid Case 1 - increment a key that doesn't exist
	PFObjectPtr scoreboard = PFObject::objectWithClassName("Scoreboard");
	QCOMPARE(scoreboard->allKeys().count(), 0);
	scoreboard->incrementKey("score");
	QCOMPARE(scoreboard->allKeys().count(), 0);

	// Invalid Case 2 - increment a key that is not a number
	scoreboard->setObjectForKey(QString("Really High"), "score");
	QCOMPARE(scoreboard->objectForKey("score").toString(), QString("Really High"));
	QCOMPARE(scoreboard->allKeys().count(), 1);
	scoreboard->incrementKey("score");
	QCOMPARE(scoreboard->objectForKey("score").toString(), QString("Really High"));
	QCOMPARE(scoreboard->allKeys().count(), 1);

	// Valid Case 1 - increment key (int) for object not in the cloud
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27, "score");
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 27);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Int);
	scoreboard->incrementKey("score");
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 28);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Int);

	// Valid Case 2 - increment key (float) for object not in the cloud
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27.085f, "score");
	QCOMPARE(scoreboard->objectForKey("score").toFloat(), 27.085f);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Float);
	scoreboard->incrementKey("score");
	QCOMPARE(scoreboard->objectForKey("score").toFloat(), 28.085f);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Float);

	// Valid Case 3 - increment key for cloud object
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27, "score");
	QCOMPARE(scoreboard->save(), true);
	scoreboard->incrementKey("score");
	QCOMPARE(scoreboard->save(), true);
	scoreboard->removeObjectForKey("score");
	QCOMPARE(scoreboard->allKeys().count(), 0);
	QCOMPARE(scoreboard->fetch(), true);
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 28);

	// Cleanup
	QCOMPARE(scoreboard->deleteObject(), true);
}

void TestPFObject::test_incrementKeyByAmount()
{
	// Invalid Case 1 - increment a key that doesn't exist
	PFObjectPtr scoreboard = PFObject::objectWithClassName("Scoreboard");
	QCOMPARE(scoreboard->allKeys().count(), 0);
	scoreboard->incrementKeyByAmount("score", 7);
	QCOMPARE(scoreboard->allKeys().count(), 0);

	// Invalid Case 2 - increment a key that is not a number
	scoreboard->setObjectForKey(QString("Really High"), "score");
	QCOMPARE(scoreboard->objectForKey("score").toString(), QString("Really High"));
	QCOMPARE(scoreboard->allKeys().count(), 1);
	scoreboard->incrementKeyByAmount("score", 10);
	QCOMPARE(scoreboard->objectForKey("score").toString(), QString("Really High"));
	QCOMPARE(scoreboard->allKeys().count(), 1);

	// Valid Case 1 - increment key (int) for object not in the cloud
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27, "score");
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 27);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Int);
	scoreboard->incrementKeyByAmount("score", 10);
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 37);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Int);

	// Valid Case 2 - increment key (float) for object not in the cloud
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27.085f, "score");
	QCOMPARE(scoreboard->objectForKey("score").toFloat(), 27.085f);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Float);
	scoreboard->incrementKeyByAmount("score", 10);
	QCOMPARE(scoreboard->objectForKey("score").toFloat(), 37.085f);
	QCOMPARE((QMetaType::Type) scoreboard->objectForKey("score").type(), QMetaType::Float);

	// Valid Case 3 - increment key for cloud object
	scoreboard->removeObjectForKey("score");
	scoreboard->setObjectForKey(27, "score");
	QCOMPARE(scoreboard->save(), true);
	scoreboard->incrementKeyByAmount("score", 10);
	QCOMPARE(scoreboard->save(), true);
	scoreboard->removeObjectForKey("score");
	QCOMPARE(scoreboard->allKeys().count(), 0);
	QCOMPARE(scoreboard->fetch(), true);
	QCOMPARE(scoreboard->objectForKey("score").toInt(), 37);

	// Cleanup
	QCOMPARE(scoreboard->deleteObject(), true);
}

void TestPFObject::test_addObjectToListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QVariantList pieces;
	pieces << QString("King");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addObjectToListForKey(pieces.at(0), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addObjectToListForKey(QString("King"), "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	chess->addObjectToListForKey(QString("Queen"), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Valid Case 2 - Object is in Cloud
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	cloudChess->addObjectToListForKey(QString("Bishop"), "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 3);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(2).toString(), QString("Bishop"));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(2).toString(), QString("Bishop"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_addObjectToListForKeyWithSerializable()
{
	// Create two chess pieces
	PFObjectPtr king = PFObject::objectWithClassName("ChessPiece");
	king->setObjectForKey(QString("King"), "name");
	QCOMPARE(king->save(), true);
	PFObjectPtr queen = PFObject::objectWithClassName("ChessPiece");
	queen->setObjectForKey(QString("Queen"), "name");
	QCOMPARE(queen->save(), true);

	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addObjectToListForKey(king, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addObjectToListForKey(king, "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	chess->setObjectForKey(QVariantList(), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 0);
	chess->addObjectToListForKey(king, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0)), king);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0))->objectId(), king->objectId());

	// Valid Case 2 - Object is in Cloud
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 1);
	cloudChess->addObjectToListForKey(queen, "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(1)), queen);

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(1))->objectId(), queen->objectId());

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
	QCOMPARE(king->deleteObject(), true);
	QCOMPARE(queen->deleteObject(), true);
}

void TestPFObject::test_addObjectsToListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QVariantList pieces;
	pieces << QString("King") << QString("Queen");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addObjectsToListForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Objects List is empty
	chess->addObjectsToListForKey(QVariantList(), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 3 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addObjectsToListForKey(QVariantList(), "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	chess->setObjectForKey(pieces, "piecesRemaining");
	QVariantList morePieces;
	morePieces << QString("Knight") << QString("Rook");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	chess->addObjectsToListForKey(morePieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(2), morePieces.at(0));

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(2), morePieces.at(0));

	// Valid Case 2 - Object is in Cloud
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QVariantList evenMorePieces;
	evenMorePieces << QString("Bishop") << QString("Pawn");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 4);
	cloudChess->addObjectsToListForKey(evenMorePieces, "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 6);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(4), evenMorePieces.at(0));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 6);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(4).toString(), QString("Bishop"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_addUniqueObjectToListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QVariantList pieces;
	pieces << QString("King");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addUniqueObjectToListForKey(pieces.at(0), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addUniqueObjectToListForKey(QString("King"), "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	chess->addUniqueObjectToListForKey(QString("Queen"), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Add a non-unique object
	chess->addUniqueObjectToListForKey(QString("King"), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Valid Case 2 - Object is in Cloud
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	cloudChess->addUniqueObjectToListForKey(QString("Queen"), "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Queen"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_addUniqueObjectToListForKeyWithSerializable()
{
	// Create four chess pieces
	PFObjectPtr king = PFObject::objectWithClassName("ChessPiece");
	king->setObjectForKey(QString("King"), "name");
	QCOMPARE(king->save(), true);
	PFObjectPtr queen = PFObject::objectWithClassName("ChessPiece");
	queen->setObjectForKey(QString("Queen"), "name");
	QCOMPARE(queen->save(), true);
	PFObjectPtr knight = PFObject::objectWithClassName("ChessPiece");
	knight->setObjectForKey(QString("Knight"), "name");
	QCOMPARE(knight->save(), true);
	PFObjectPtr bishop = PFObject::objectWithClassName("ChessPiece");
	bishop->setObjectForKey(QString("Bishop"), "name");
	QCOMPARE(bishop->save(), true);

	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addUniqueObjectToListForKey(king, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addUniqueObjectToListForKey(king, "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	QVariantList pieces;
	pieces << PFObject::toVariant(king) << PFObject::toVariant(queen);
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	chess->addUniqueObjectToListForKey(king, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0)), king);

	// Add a unique object
	chess->addUniqueObjectToListForKey(bishop, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(2)), bishop);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0))->objectId(), king->objectId());
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(1))->objectId(), queen->objectId());
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(2))->objectId(), bishop->objectId());

	// Valid Case 2 - Grab the cloud object and add the knight to it
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 3);
	cloudChess->addUniqueObjectToListForKey(queen, "piecesRemaining"); // Will fail b/c already added
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 3);
	cloudChess->addUniqueObjectToListForKey(knight, "piecesRemaining"); // Will succeed
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(0))->objectId(), king->objectId());
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(1))->objectId(), queen->objectId());
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(2))->objectId(), bishop->objectId());
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(3))->objectId(), knight->objectId());

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0))->objectId(), king->objectId());
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(1))->objectId(), queen->objectId());
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(2))->objectId(), bishop->objectId());
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(3))->objectId(), knight->objectId());

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
	QCOMPARE(king->deleteObject(), true);
	QCOMPARE(queen->deleteObject(), true);
	QCOMPARE(knight->deleteObject(), true);
	QCOMPARE(bishop->deleteObject(), true);
}

void TestPFObject::test_addUniqueObjectsToListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QVariantList pieces;
	pieces << QString("King") << QString("Queen");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->addUniqueObjectsToListForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 2 - Objects List is empty
	chess->addUniqueObjectsToListForKey(QVariantList(), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Invalid Case 3 - Object For Key is NOT a List
	chess->setObjectForKey(QString("Chess"), "name");
	QCOMPARE(chess->allKeys().count(), 1);
	chess->addUniqueObjectsToListForKey(QVariantList(), "name");
	QCOMPARE((QMetaType::Type) chess->objectForKey("name").type(), QMetaType::QString);
	QCOMPARE(chess->allKeys().count(), 1);

	// Valid Case 1 - Object is New
	chess->setObjectForKey(pieces, "piecesRemaining");
	QVariantList morePieces;
	morePieces << QString("Knight") << QString("Rook") << QString("King") << QString("Queen");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	chess->addUniqueObjectsToListForKey(morePieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(2), morePieces.at(0));

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 4);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(2), morePieces.at(0));

	// Valid Case 2 - Object is in Cloud
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QVariantList evenMorePieces;
	evenMorePieces << QString("Bishop") << QString("Pawn") << QString("Rook") << QString("King");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 4);
	cloudChess->addUniqueObjectsToListForKey(evenMorePieces, "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 6);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(4), evenMorePieces.at(0));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 6);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(4).toString(), QString("Bishop"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_removeObjectFromListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->removeObjectFromListForKey(QVariant(), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Add some pieces to the array
	QVariantList pieces;
	pieces << QString("King") << QString("Queen") << QString("Knight");
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 1);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Try to remove a piece that isn't in the array
	chess->removeObjectFromListForKey(QString("Rook"), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Remove an object that is stored inside the array
	chess->removeObjectFromListForKey(QString("Queen"), "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Knight"));

	// Pull the object from the cloud and remove a piece
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	cloudChess->removeObjectFromListForKey(QString("King"), "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(0).toString(), QString("Knight"));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(0).toString(), QString("Knight"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_removeObjectFromListForKeyWithSerializable()
{
	// Create four chess pieces
	PFObjectPtr king = PFObject::objectWithClassName("ChessPiece");
	king->setObjectForKey(QString("King"), "name");
	QCOMPARE(king->save(), true);
	PFObjectPtr queen = PFObject::objectWithClassName("ChessPiece");
	queen->setObjectForKey(QString("Queen"), "name");
	QCOMPARE(queen->save(), true);
	PFObjectPtr knight = PFObject::objectWithClassName("ChessPiece");
	knight->setObjectForKey(QString("Knight"), "name");
	QCOMPARE(knight->save(), true);
	PFObjectPtr bishop = PFObject::objectWithClassName("ChessPiece");
	bishop->setObjectForKey(QString("Bishop"), "name");
	QCOMPARE(bishop->save(), true);

	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->removeObjectFromListForKey(king, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Add some pieces to the array
	QVariantList pieces;
	pieces << PFObject::toVariant(king) << PFObject::toVariant(queen) << PFObject::toVariant(knight);
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 1);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Try to remove a piece that isn't in the array
	chess->removeObjectFromListForKey(bishop, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Remove an object that is stored inside the array
	chess->removeObjectFromListForKey(queen, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(1))->objectId(), knight->objectId());

	// Pull the object from the cloud and remove a piece
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	cloudChess->removeObjectFromListForKey(king, "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(PFObject::objectFromVariant(cloudChess->objectForKey("piecesRemaining").toList().at(0))->objectId(), knight->objectId());

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(PFObject::objectFromVariant(chess->objectForKey("piecesRemaining").toList().at(0))->objectId(), knight->objectId());

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
	QCOMPARE(king->deleteObject(), true);
	QCOMPARE(queen->deleteObject(), true);
	QCOMPARE(knight->deleteObject(), true);
	QCOMPARE(bishop->deleteObject(), true);
}

void TestPFObject::test_removeObjectsFromListForKey()
{
	// Invalid Case 1 - Object does NOT contain key
	PFObjectPtr chess = PFObject::objectWithClassName("Game");
	QCOMPARE(chess->allKeys().count(), 0);
	chess->removeObjectsFromListForKey(QVariantList(), "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 0);

	// Add some pieces to the array
	QVariantList pieces;
	pieces << QString("King") << QString("Queen") << QString("Knight");
	chess->setObjectForKey(pieces, "piecesRemaining");
	QCOMPARE(chess->allKeys().count(), 1);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Try to remove pieces that aren't in the array
	QVariantList removalPieces;
	removalPieces << QString("Rook") << QString("Pawn");
	chess->removeObjectsFromListForKey(removalPieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 3);

	// Remove an object that is stored inside the array
	removalPieces.clear();
	removalPieces << QString("Queen");
	chess->removeObjectsFromListForKey(removalPieces, "piecesRemaining");
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);

	// Save the object, fetch it, and re-test
	QCOMPARE(chess->save(), true);
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 2);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(1).toString(), QString("Knight"));

	// Pull the object from the cloud and remove a piece
	PFObjectPtr cloudChess = PFObject::objectWithClassName(chess->className(), chess->objectId());
	QCOMPARE(cloudChess->fetch(), true);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 2);
	removalPieces.clear();
	removalPieces << QString("King");
	cloudChess->removeObjectsFromListForKey(removalPieces, "piecesRemaining");
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(cloudChess->objectForKey("piecesRemaining").toList().at(0).toString(), QString("Knight"));

	// Save the cloud object
	QCOMPARE(cloudChess->save(), true);

	// Fetch the original chess object and re-test
	QCOMPARE(chess->fetch(), true);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().count(), 1);
	QCOMPARE(chess->objectForKey("piecesRemaining").toList().at(0).toString(), QString("Knight"));

	// Cleanup
	QCOMPARE(chess->deleteObject(), true);
}

void TestPFObject::test_setACL()
{
	// Create a level and verify the ACL is empty by default
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->ACL().isNull(), true);

	// Add an ACL and verify it is set properly and also added to the objects list
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	level->setACL(acl);
	QCOMPARE(level->ACL().isNull(), false);
	QCOMPARE(level->ACL()->publicReadAccess(), true);
	QCOMPARE(level->ACL()->publicWriteAccess(), false);

	// Reset the ACL
	QCOMPARE(level->allKeys().toSet().contains("ACL"), true);
	level->setACL(PFACLPtr());
	QCOMPARE(level->ACL().isNull(), true);
	QCOMPARE(level->allKeys().toSet().contains("ACL"), false);
}

void TestPFObject::test_ACL()
{
	// Create a level and verify the ACL is empty by default
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->ACL().isNull(), true);

	// Add an ACL and verify it is set properly and also added to the objects list
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	level->setACL(acl);
	QCOMPARE(level->ACL().isNull(), false);
	QCOMPARE(level->ACL()->publicReadAccess(), true);
	QCOMPARE(level->ACL()->publicWriteAccess(), false);

	// Reset the ACL
	QCOMPARE(level->allKeys().toSet().contains("ACL"), true);
	level->setACL(PFACLPtr());
	QCOMPARE(level->ACL().isNull(), true);
	QCOMPARE(level->allKeys().toSet().contains("ACL"), false);
}

void TestPFObject::test_parseClassName()
{
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->className(), QString("Level"));
}

void TestPFObject::test_objectId()
{
	// Non-uploaded object
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->objectId().isEmpty(), true);

	// Upload the object
	bool saved = level->save();
	QCOMPARE(saved, true);

	// Retest the object id
	QCOMPARE(level->objectId().isEmpty(), false);

	// Cleanup the object from the cloud
	bool deleted = level->deleteObject();
	QCOMPARE(deleted, true);
}

void TestPFObject::test_createdAt()
{
	// Non-uploaded object
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->createdAt().isNull(), true);

	// Upload the object
	bool saved = level->save();
	QCOMPARE(saved, true);

	// Retest the created at ivar
	QCOMPARE(level->createdAt().isNull(), false);
	QCOMPARE(level->createdAt()->dateTime().toLocalTime().date(), QDate::currentDate());

	// Cleanup the object from the cloud
	bool deleted = level->deleteObject();
	QCOMPARE(deleted, true);
}

void TestPFObject::test_updatedAt()
{
	// Non-uploaded object
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->updatedAt().isNull(), true);

	// Upload the object
	bool saved = level->save();
	QCOMPARE(saved, true);

	// Retest the updated at ivar (should still be NULL)
	QCOMPARE(level->updatedAt().isNull(), true);

	// Update the object
	level->setObjectForKey(QString("Act I"), "name");
	bool updated = level->save();
	QCOMPARE(updated, true);

	// Retest the updated at ivar (should be valid now)
	QCOMPARE(level->updatedAt().isNull(), false);
	QCOMPARE(level->updatedAt()->dateTime().toLocalTime().date(), QDate::currentDate());

	// Cleanup the object from the cloud
	bool deleted = level->deleteObject();
	QCOMPARE(deleted, true);
}

void TestPFObject::test_save()
{
	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save them both
	bool diabloSaved = diablo->save();
	bool wizardSaved = wizard->save();
	QCOMPARE(diabloSaved, true);
	QCOMPARE(wizardSaved, true);

	// Create an map image file
	QString filename = "small_image.jpg";
	QString filepath = QDir(_dataPath).absoluteFilePath(filename);
	PFFilePtr imageFile = PFFile::fileWithNameAndContentsAtPath(filename, filepath);
	bool imageFileSaved = imageFile->save();
	QCOMPARE(imageFileSaved, true);

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");
	map->setObjectForKey(imageFile, "mapImage");

	// Save the map
	bool mapSaved = map->save();
	QCOMPARE(mapSaved, true);

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	bool levelSaved = level->save();
	QCOMPARE(levelSaved, true);

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");
	bool levelUpdated = level->save();
	QCOMPARE(levelUpdated, true);

	// Cleanup
	bool levelDeleted = level->deleteObject();
	bool mapDeleted = map->deleteObject();
	bool diabloDeleted = diablo->deleteObject();
	bool wizardDeleted = wizard->deleteObject();
	QCOMPARE(levelDeleted, true);
	QCOMPARE(mapDeleted, true);
	QCOMPARE(diabloDeleted, true);
	QCOMPARE(wizardDeleted, true);
}

void TestPFObject::test_saveWithError()
{
	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save them both
	PFErrorPtr diabloSaveError, wizardSaveError;
	bool diabloSaved = diablo->save(diabloSaveError);
	bool wizardSaved = wizard->save(wizardSaveError);
	QCOMPARE(diabloSaved, true);
	QCOMPARE(wizardSaved, true);
	QCOMPARE(diabloSaveError.isNull(), true);
	QCOMPARE(wizardSaveError.isNull(), true);

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");

	// Save the map
	PFErrorPtr mapSaveError;
	bool mapSaved = map->save(mapSaveError);
	QCOMPARE(mapSaved, true);
	QCOMPARE(mapSaveError.isNull(), true);

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	PFErrorPtr levelSaveError;
	bool levelSaved = level->save(levelSaveError);
	QCOMPARE(levelSaved, true);
	QCOMPARE(levelSaveError.isNull(), true);

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");
	PFErrorPtr levelUpdateError;
	bool levelUpdated = level->save(levelUpdateError);
	QCOMPARE(levelUpdated, true);
	QCOMPARE(levelUpdateError.isNull(), true);

	// Cleanup
	bool levelDeleted = level->deleteObject();
	bool mapDeleted = map->deleteObject();
	bool diabloDeleted = diablo->deleteObject();
	bool wizardDeleted = wizard->deleteObject();
	QCOMPARE(levelDeleted, true);
	QCOMPARE(mapDeleted, true);
	QCOMPARE(diabloDeleted, true);
	QCOMPARE(wizardDeleted, true);
}

void TestPFObject::test_saveInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save diablo
	bool saveStarted = diablo->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(saveStarted, true);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError.isNull(), true);

	// Reset flags
	_saveSucceeded = false;
	_saveError = PFErrorPtr();

	// Save the wizard
	saveStarted = wizard->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(saveStarted, true);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError.isNull(), true);

	// Reset flags
	_saveSucceeded = false;
	_saveError = PFErrorPtr();

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");

	// Save the map
	saveStarted = map->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(saveStarted, true);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError.isNull(), true);

	// Reset flags
	_saveSucceeded = false;
	_saveError = PFErrorPtr();

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	saveStarted = level->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(saveStarted, true);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError.isNull(), true);

	// Reset flags
	_saveSucceeded = false;
	_saveError = PFErrorPtr();

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");

	// Resave the level
	saveStarted = level->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(saveStarted, true);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError.isNull(), true);

	// Cleanup
	bool levelDeleted = level->deleteObject();
	bool mapDeleted = map->deleteObject();
	bool diabloDeleted = diablo->deleteObject();
	bool wizardDeleted = wizard->deleteObject();
	QCOMPARE(levelDeleted, true);
	QCOMPARE(mapDeleted, true);
	QCOMPARE(diabloDeleted, true);
	QCOMPARE(wizardDeleted, true);
}

void TestPFObject::test_deleteObject()
{
	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save them both
	bool diabloSaved = diablo->save();
	bool wizardSaved = wizard->save();
	QCOMPARE(diabloSaved, true);
	QCOMPARE(wizardSaved, true);

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");

	// Save the map
	bool mapSaved = map->save();
	QCOMPARE(mapSaved, true);

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	bool levelSaved = level->save();
	QCOMPARE(levelSaved, true);

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");
	bool levelUpdated = level->save();
	QCOMPARE(levelUpdated, true);

	// Cleanup
	bool levelDeleted = level->deleteObject();
	bool mapDeleted = map->deleteObject();
	bool diabloDeleted = diablo->deleteObject();
	bool wizardDeleted = wizard->deleteObject();
	QCOMPARE(levelDeleted, true);
	QCOMPARE(mapDeleted, true);
	QCOMPARE(diabloDeleted, true);
	QCOMPARE(wizardDeleted, true);
}

void TestPFObject::test_deleteObjectWithError()
{
	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save them both
	bool diabloSaved = diablo->save();
	bool wizardSaved = wizard->save();
	QCOMPARE(diabloSaved, true);
	QCOMPARE(wizardSaved, true);

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");

	// Save the map
	bool mapSaved = map->save();
	QCOMPARE(mapSaved, true);

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	bool levelSaved = level->save();
	QCOMPARE(levelSaved, true);

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");
	bool levelUpdated = level->save();
	QCOMPARE(levelUpdated, true);

	// Cleanup
	PFErrorPtr levelDeletedError, mapDeletedError, diabloDeletedError, wizardDeletedError;
	bool levelDeleted = level->deleteObject(levelDeletedError);
	bool mapDeleted = map->deleteObject(mapDeletedError);
	bool diabloDeleted = diablo->deleteObject(diabloDeletedError);
	bool wizardDeleted = wizard->deleteObject(wizardDeletedError);
	QCOMPARE(levelDeleted, true);
	QCOMPARE(mapDeleted, true);
	QCOMPARE(diabloDeleted, true);
	QCOMPARE(wizardDeleted, true);
	QCOMPARE(levelDeletedError.isNull(), true);
	QCOMPARE(mapDeletedError.isNull(), true);
	QCOMPARE(diabloDeletedError.isNull(), true);
	QCOMPARE(wizardDeletedError.isNull(), true);
}

void TestPFObject::test_deleteObjectInBackground()
{
	// Create two characters
	PFObjectPtr diablo = PFObject::objectWithClassName("Character");
	diablo->setObjectForKey(QString("Diablo"), "name");
	diablo->setObjectForKey(QString("Bad Guy"), "characterType");
	diablo->setObjectForKey(80, "power");
	PFObjectPtr wizard = PFObject::objectWithClassName("Character");
	wizard->setObjectForKey(QString("Jerry"), "name");
	wizard->setObjectForKey(QString("Wizard"), "characterType");
	wizard->setObjectForKey(100, "power");

	// Save them both
	bool diabloSaved = diablo->save();
	bool wizardSaved = wizard->save();
	QCOMPARE(diabloSaved, true);
	QCOMPARE(wizardSaved, true);

	// Create a map
	PFObjectPtr map = PFObject::objectWithClassName("Map");
	map->setObjectForKey(1200, "width");
	map->setObjectForKey(800, "height");

	// Save the map
	bool mapSaved = map->save();
	QCOMPARE(mapSaved, true);

	// Create a level with both characters and the map
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	level->setObjectForKey(QString("Act I"), "name");
	QVariantList characters;
	characters << PFObject::toVariant(diablo) << PFObject::toVariant(wizard);
	level->setObjectForKey(characters, "characters");
	level->setObjectForKey(map, "map");

	// Add some other attributes to the level
	PFDateTimePtr upgradeDate = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	level->setObjectForKey(upgradeDate, "upgradeDate");
	level->setObjectForKey(0.4879, "progress");
	PFACLPtr acl = PFACL::ACL();
	acl->setPublicReadAccess(true);
	acl->setPublicWriteAccess(true);
	level->setACL(acl);

	// Save the level
	bool levelSaved = level->save();
	QCOMPARE(levelSaved, true);

	// Update the level name
	level->setObjectForKey(QString("Act II"), "name");
	bool levelUpdated = level->save();
	QCOMPARE(levelUpdated, true);

	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Delete the level
	bool deleteObjectStarted = level->deleteObjectInBackground(this, SLOT(deleteObjectCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(deleteObjectEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(deleteObjectStarted, true);
	QCOMPARE(_deleteObjectSucceeded, true);
	QCOMPARE(_deleteObjectError.isNull(), true);

	// Reset flags
	_deleteObjectSucceeded = false;
	_deleteObjectError = PFErrorPtr();

	// Delete the map
	deleteObjectStarted = map->deleteObjectInBackground(this, SLOT(deleteObjectCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(deleteObjectEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(deleteObjectStarted, true);
	QCOMPARE(_deleteObjectSucceeded, true);
	QCOMPARE(_deleteObjectError.isNull(), true);

	// Reset flags
	_deleteObjectSucceeded = false;
	_deleteObjectError = PFErrorPtr();

	// Delete diablo
	deleteObjectStarted = diablo->deleteObjectInBackground(this, SLOT(deleteObjectCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(deleteObjectEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(deleteObjectStarted, true);
	QCOMPARE(_deleteObjectSucceeded, true);
	QCOMPARE(_deleteObjectError.isNull(), true);

	// Reset flags
	_deleteObjectSucceeded = false;
	_deleteObjectError = PFErrorPtr();

	// Delete the wizard
	deleteObjectStarted = wizard->deleteObjectInBackground(this, SLOT(deleteObjectCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(deleteObjectEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(deleteObjectStarted, true);
	QCOMPARE(_deleteObjectSucceeded, true);
	QCOMPARE(_deleteObjectError.isNull(), true);
}

void TestPFObject::test_isDataAvailable()
{
	// Case 1 - new object returns true
	PFObjectPtr newObject = PFObject::objectWithClassName("NiceAndShiny");
	QCOMPARE(newObject->isDataAvailable(), true);

	// Case 2 - fetched object returns true
	PFObjectPtr axe = PFObject::objectWithClassName(_axe->className(), _axe->objectId());
	QCOMPARE(axe->isDataAvailable(), false);
	QCOMPARE(axe->fetch(), true);
	QCOMPARE(axe->isDataAvailable(), true);

	// Case 3 - an unfetched object with an object id returns false
	PFObjectPtr fakeObject = PFObject::objectWithClassName("Faker", "s34af34a3f");
	QCOMPARE(fakeObject->isDataAvailable(), false);
}

void TestPFObject::test_fetch()
{
	//================================================================
	//                        Invalid Cases
	//================================================================

	// Invalid Case 1 - already fetching (fully tested in test_fetchInBackground())

	// Invalid Case 2 - try to fetch an object that hasn't been put into the cloud
	PFObjectPtr invalidObject = PFObject::objectWithClassName("SimplyBad");
	QCOMPARE(invalidObject->fetch(), false);

	//================================================================
	//                         Valid Cases
	//================================================================

	// Fetch the hero
	PFObjectPtr hero = PFObject::objectWithClassName("Character", _hero->objectId());
	QCOMPARE(hero->fetch(), true);

	// Test hero primitive objects
	QCOMPARE(hero->objectForKey("name").toString(), QString("Hercules"));
	QCOMPARE(hero->objectForKey("characterType").toString(), QString("Hero"));
	QCOMPARE(hero->objectForKey("power").toInt(), 80);
	QCOMPARE(hero->createdAt()->dateTime(), _hero->createdAt()->dateTime());

	// PFDateTime (test hero expirationDate)
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromVariant(hero->objectForKey("expirationDate"));
	QCOMPARE(expirationDate->dateTime(), _expirationDate->dateTime());

	// PFFile (test hero tutorialFile)
	PFFilePtr tutorialFile = PFFile::fileFromVariant(hero->objectForKey("tutorialFile"));
	QCOMPARE(tutorialFile->url(), _tutorialFile->url());

	// PFACL (test hero ACL)
	QCOMPARE(hero->ACL().isNull(), _hero->ACL().isNull());
	QCOMPARE(hero->ACL()->publicReadAccess(), _hero->ACL()->publicReadAccess());
	QCOMPARE(hero->ACL()->publicWriteAccess(), _hero->ACL()->publicWriteAccess());
	QCOMPARE(hero->ACL()->publicReadAccess(), true);
	QCOMPARE(hero->ACL()->publicWriteAccess(), true);

	// PFUser (test hero accountOwner - need to fetch)
	PFUserPtr testUser = PFUser::userFromVariant(hero->objectForKey("accountOwner"));
	bool testUserFetched = testUser->fetch();
	QCOMPARE(testUserFetched, true);
	QCOMPARE(testUser->username(), _testUser->username());
	QCOMPARE(testUser->email(), _testUser->email());

	// PFObject (test hero sidekick - need to fetch)
	PFObjectPtr sidekick = PFObject::objectFromVariant(hero->objectForKey("sidekick"));
	bool sidekickFetched = sidekick->fetch();
	QCOMPARE(sidekickFetched, true);
	QCOMPARE(sidekick->objectForKey("name").toString(), QString("Menalous"));
	QCOMPARE(sidekick->objectForKey("characterType").toString(), QString("Sidekick"));
	QCOMPARE(sidekick->objectForKey("power").toInt(), 24);

	// PFObject List (test hero weapons - need to fetch)
	QVariantList weapons = hero->objectForKey("weapons").toList();
	PFObjectPtr axe = PFObject::objectFromVariant(weapons.at(0));
	PFObjectPtr sword = PFObject::objectFromVariant(weapons.at(1));
	bool axeFetched = axe->fetch();
	QCOMPARE(axeFetched, true);
	bool swordFetched = sword->fetch();
	QCOMPARE(swordFetched, true);
	QCOMPARE(axe->objectForKey("attackMin").toInt(), 19);
	QCOMPARE(axe->objectForKey("attackMax").toInt(), 26);
	QCOMPARE(axe->objectForKey("strengthRequired").toInt(), 65);
	QCOMPARE(sword->objectForKey("attackMin").toInt(), 13);
	QCOMPARE(sword->objectForKey("attackMax").toInt(), 15);
	QCOMPARE(sword->objectForKey("strengthRequired").toInt(), 29);

	// PFObject Map (test hero armor - need to fetch)
	QVariantMap armor = hero->objectForKey("armor").toMap();
	PFObjectPtr greaves = PFObject::objectFromVariant(armor["greaves"]);
	PFObjectPtr helmet = PFObject::objectFromVariant(armor["helmet"]);
	bool greavesFetched = greaves->fetch();
	QCOMPARE(greavesFetched, true);
	bool helmetFetched = helmet->fetch();
	QCOMPARE(helmetFetched, true);
	QCOMPARE(greaves->objectForKey("name").toString(), QString("Greaves"));
	QCOMPARE(greaves->objectForKey("defense").toInt(), 26);
	QCOMPARE(helmet->objectForKey("name").toString(), QString("Helmet"));
	QCOMPARE(helmet->objectForKey("defense").toInt(), 121);
}

void TestPFObject::test_fetchWithError()
{
	//================================================================
	//                        Invalid Cases
	//================================================================

	// Invalid Case 1 - already fetching (fully tested in test_fetchInBackground())

	// Invalid Case 2 - try to fetch an object that hasn't been put into the cloud
	PFObjectPtr invalidObject = PFObject::objectWithClassName("SimplyBad");
	PFErrorPtr invalidObjectFetchError;
	QCOMPARE(invalidObject->fetch(invalidObjectFetchError), false);
	QCOMPARE(invalidObjectFetchError.isNull(), true);

	//================================================================
	//                         Valid Cases
	//================================================================

	// Fetch the hero
	PFObjectPtr hero = PFObject::objectWithClassName("Character", _hero->objectId());
	PFErrorPtr heroFetchError;
	bool heroFetched = hero->fetch(heroFetchError);
	QCOMPARE(heroFetched, true);
	QCOMPARE(heroFetchError.isNull(), true);

	// Test hero primitive objects
	QCOMPARE(hero->objectForKey("name").toString(), QString("Hercules"));
	QCOMPARE(hero->objectForKey("characterType").toString(), QString("Hero"));
	QCOMPARE(hero->objectForKey("power").toInt(), 80);
	QCOMPARE(hero->createdAt()->dateTime(), _hero->createdAt()->dateTime());

	// PFDateTime (test hero expirationDate)
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromVariant(hero->objectForKey("expirationDate"));
	QCOMPARE(expirationDate->dateTime(), _expirationDate->dateTime());

	// PFFile (test hero tutorialFile)
	PFFilePtr tutorialFile = PFFile::fileFromVariant(hero->objectForKey("tutorialFile"));
	QCOMPARE(tutorialFile->url(), _tutorialFile->url());

	// PFACL (test hero ACL)
	QCOMPARE(hero->ACL().isNull(), _hero->ACL().isNull());
	QCOMPARE(hero->ACL()->publicReadAccess(), _hero->ACL()->publicReadAccess());
	QCOMPARE(hero->ACL()->publicWriteAccess(), _hero->ACL()->publicWriteAccess());
	QCOMPARE(hero->ACL()->publicReadAccess(), true);
	QCOMPARE(hero->ACL()->publicWriteAccess(), true);

	// PFUser (test hero accountOwner - need to fetch)
	PFUserPtr testUser = PFUser::userFromVariant(hero->objectForKey("accountOwner"));
	PFErrorPtr testUserFetchError;
	bool testUserFetched = testUser->fetch(testUserFetchError);
	QCOMPARE(testUserFetched, true);
	QCOMPARE(testUserFetchError.isNull(), true);
	QCOMPARE(testUser->username(), _testUser->username());
	QCOMPARE(testUser->email(), _testUser->email());

	// PFObject (test hero sidekick - need to fetch)
	PFObjectPtr sidekick = PFObject::objectFromVariant(hero->objectForKey("sidekick"));
	PFErrorPtr sidekickFetchError;
	bool sidekickFetched = sidekick->fetch(sidekickFetchError);
	QCOMPARE(sidekickFetched, true);
	QCOMPARE(sidekickFetchError.isNull(), true);
	QCOMPARE(sidekick->objectForKey("name").toString(), QString("Menalous"));
	QCOMPARE(sidekick->objectForKey("characterType").toString(), QString("Sidekick"));
	QCOMPARE(sidekick->objectForKey("power").toInt(), 24);

	// PFObject List (test hero weapons - need to fetch)
	QVariantList weapons = hero->objectForKey("weapons").toList();
	PFObjectPtr axe = PFObject::objectFromVariant(weapons.at(0));
	PFObjectPtr sword = PFObject::objectFromVariant(weapons.at(1));
	PFErrorPtr axeFetchError;
	bool axeFetched = axe->fetch(axeFetchError);
	QCOMPARE(axeFetched, true);
	QCOMPARE(axeFetchError.isNull(), true);
	PFErrorPtr swordFetchError;
	bool swordFetched = sword->fetch(swordFetchError);
	QCOMPARE(swordFetched, true);
	QCOMPARE(swordFetchError.isNull(), true);
	QCOMPARE(axe->objectForKey("attackMin").toInt(), 19);
	QCOMPARE(axe->objectForKey("attackMax").toInt(), 26);
	QCOMPARE(axe->objectForKey("strengthRequired").toInt(), 65);
	QCOMPARE(sword->objectForKey("attackMin").toInt(), 13);
	QCOMPARE(sword->objectForKey("attackMax").toInt(), 15);
	QCOMPARE(sword->objectForKey("strengthRequired").toInt(), 29);

	// PFObject Map (test hero armor - need to fetch)
	QVariantMap armor = hero->objectForKey("armor").toMap();
	PFObjectPtr greaves = PFObject::objectFromVariant(armor["greaves"]);
	PFObjectPtr helmet = PFObject::objectFromVariant(armor["helmet"]);
	PFErrorPtr greavesFetchError;
	bool greavesFetched = greaves->fetch(greavesFetchError);
	QCOMPARE(greavesFetched, true);
	QCOMPARE(greavesFetchError.isNull(), true);
	PFErrorPtr helmetFetchError;
	bool helmetFetched = helmet->fetch(helmetFetchError);
	QCOMPARE(helmetFetched, true);
	QCOMPARE(helmetFetchError.isNull(), true);
	QCOMPARE(greaves->objectForKey("name").toString(), QString("Greaves"));
	QCOMPARE(greaves->objectForKey("defense").toInt(), 26);
	QCOMPARE(helmet->objectForKey("name").toString(), QString("Helmet"));
	QCOMPARE(helmet->objectForKey("defense").toInt(), 121);
}

void TestPFObject::test_fetchInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	//================================================================
	//                        Invalid Cases
	//================================================================

	////////////////////////////////////////////////////////////////////////////////
	// Invalid Case 1 - already fetching
	////////////////////////////////////////////////////////////////////////////////

	PFObjectPtr hammer = PFObject::objectWithClassName("Tool");
	hammer->setObjectForKey(QString("Hammer"), "name");
	QCOMPARE(hammer->save(), true);
	PFObjectPtr cloudHammer = PFObject::objectWithClassName("Tool", hammer->objectId());
	QCOMPARE(cloudHammer->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);

	// Try to fetch all three different ways (all should fail b/c the previous fetch is already in progress)
	QCOMPARE(cloudHammer->fetch(), false); // fails b/c fetch in progress
	PFErrorPtr hammerFetchError;
	QCOMPARE(cloudHammer->fetch(hammerFetchError), false); // fails b/c fetch in progress
	QCOMPARE(hammerFetchError.isNull(), true);
	QCOMPARE(cloudHammer->fetchInBackground(NULL, 0), false); // fails b/c fetch in progress

	// Wait until the fetch reply is captured then test to make sure the first fetch succeeded
	QObject::connect(this, SIGNAL(fetchEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Delete the hammer from the cloud
	QCOMPARE(hammer->deleteObject(), true);

	////////////////////////////////////////////////////////////////////////////////
	// Invalid Case 2 - try to fetch an object that hasn't been put into the cloud
	////////////////////////////////////////////////////////////////////////////////

	PFObjectPtr invalidObject = PFObject::objectWithClassName("LetsFail");
	QCOMPARE(invalidObject->fetchInBackground(NULL, 0), false);

	//================================================================
	//                         Valid Cases
	//================================================================

	// Fetch the hero
	PFObjectPtr hero = PFObject::objectWithClassName("Character", _hero->objectId());
	QCOMPARE(hero->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Test hero primitive objects
	QCOMPARE(hero->objectForKey("name").toString(), QString("Hercules"));
	QCOMPARE(hero->objectForKey("characterType").toString(), QString("Hero"));
	QCOMPARE(hero->objectForKey("power").toInt(), 80);
	QCOMPARE(hero->createdAt()->dateTime(), _hero->createdAt()->dateTime());

	// PFDateTime (test hero expirationDate)
	PFDateTimePtr expirationDate = PFDateTime::dateTimeFromVariant(hero->objectForKey("expirationDate"));
	QCOMPARE(expirationDate->dateTime(), _expirationDate->dateTime());

	// PFFile (test hero tutorialFile)
	PFFilePtr tutorialFile = PFFile::fileFromVariant(hero->objectForKey("tutorialFile"));
	QCOMPARE(tutorialFile->url(), _tutorialFile->url());

	// PFACL (test hero ACL)
	QCOMPARE(hero->ACL().isNull(), _hero->ACL().isNull());
	QCOMPARE(hero->ACL()->publicReadAccess(), _hero->ACL()->publicReadAccess());
	QCOMPARE(hero->ACL()->publicWriteAccess(), _hero->ACL()->publicWriteAccess());
	QCOMPARE(hero->ACL()->publicReadAccess(), true);
	QCOMPARE(hero->ACL()->publicWriteAccess(), true);

	// PFUser (test hero accountOwner - need to fetch)
	PFUserPtr testUser = PFUser::userFromVariant(hero->objectForKey("accountOwner"));
	QCOMPARE(testUser->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();
	QCOMPARE(testUser->username(), _testUser->username());
	QCOMPARE(testUser->email(), _testUser->email());

	// PFObject (test hero sidekick - need to fetch)
	PFObjectPtr sidekick = PFObject::objectFromVariant(hero->objectForKey("sidekick"));
	QCOMPARE(sidekick->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();
	QCOMPARE(sidekick->objectForKey("name").toString(), QString("Menalous"));
	QCOMPARE(sidekick->objectForKey("characterType").toString(), QString("Sidekick"));
	QCOMPARE(sidekick->objectForKey("power").toInt(), 24);

	// PFObject List (test hero weapons - need to fetch)
	QVariantList weapons = hero->objectForKey("weapons").toList();
	PFObjectPtr axe = PFObject::objectFromVariant(weapons.at(0));
	PFObjectPtr sword = PFObject::objectFromVariant(weapons.at(1));

	// Async axe fetch
	QCOMPARE(axe->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Async sword fetch
	QCOMPARE(sword->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true); // fetch axe
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Axe and sword tests
	QCOMPARE(axe->objectForKey("attackMin").toInt(), 19);
	QCOMPARE(axe->objectForKey("attackMax").toInt(), 26);
	QCOMPARE(axe->objectForKey("strengthRequired").toInt(), 65);
	QCOMPARE(sword->objectForKey("attackMin").toInt(), 13);
	QCOMPARE(sword->objectForKey("attackMax").toInt(), 15);
	QCOMPARE(sword->objectForKey("strengthRequired").toInt(), 29);

	// PFObject Map (test hero armor - need to fetch)
	QVariantMap armor = hero->objectForKey("armor").toMap();
	PFObjectPtr greaves = PFObject::objectFromVariant(armor["greaves"]);
	PFObjectPtr helmet = PFObject::objectFromVariant(armor["helmet"]);

	// Async greaves fetch
	QCOMPARE(greaves->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Async helmet fetch
	QCOMPARE(helmet->fetchInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();

	// Helmet and greaves tests
	QCOMPARE(greaves->objectForKey("name").toString(), QString("Greaves"));
	QCOMPARE(greaves->objectForKey("defense").toInt(), 26);
	QCOMPARE(helmet->objectForKey("name").toString(), QString("Helmet"));
	QCOMPARE(helmet->objectForKey("defense").toInt(), 121);
}

void TestPFObject::test_fetchIfNeeded()
{
	// Case 1 - new object won't fetch
	PFObjectPtr newObject = PFObject::objectWithClassName("NiceAndShiny");
	QCOMPARE(newObject->isDataAvailable(), true);
	QCOMPARE(newObject->fetchIfNeeded(), false);
	QCOMPARE(newObject->isDataAvailable(), true);

	// Case 2 - server object fetch if needed
	PFObjectPtr axe = PFObject::objectWithClassName(_axe->className(), _axe->objectId());
	QCOMPARE(axe->isDataAvailable(), false);
	QCOMPARE(axe->fetchIfNeeded(), true);
	QCOMPARE(axe->isDataAvailable(), true);
	QCOMPARE(axe->fetchIfNeeded(), false);
	QCOMPARE(axe->objectForKey("weaponClass").toString(), QString("Axe"));

	// Case 3 - fake object will fail in the fetch
	PFObjectPtr fakeObject = PFObject::objectWithClassName("Faker", "s34af34a3f");
	QCOMPARE(fakeObject->isDataAvailable(), false);
	QCOMPARE(fakeObject->fetchIfNeeded(), false);
}

void TestPFObject::test_fetchIfNeededWithError()
{
	// Case 1 - new object won't fetch
	PFObjectPtr newObject = PFObject::objectWithClassName("NiceAndShiny");
	QCOMPARE(newObject->isDataAvailable(), true);
	PFErrorPtr newObjectFetchError;
	QCOMPARE(newObject->fetchIfNeeded(newObjectFetchError), false);
	QCOMPARE(newObjectFetchError.isNull(), true);
	QCOMPARE(newObject->isDataAvailable(), true);

	// Case 2 - server object fetch if needed
	PFObjectPtr axe = PFObject::objectWithClassName(_axe->className(), _axe->objectId());
	QCOMPARE(axe->isDataAvailable(), false);
	PFErrorPtr axeFetchError;
	QCOMPARE(axe->fetchIfNeeded(axeFetchError), true);
	QCOMPARE(axeFetchError.isNull(), true);
	QCOMPARE(axe->isDataAvailable(), true);
	PFErrorPtr axeFetchError2;
	QCOMPARE(axe->fetchIfNeeded(axeFetchError2), false);
	QCOMPARE(axeFetchError2.isNull(), true);
	QCOMPARE(axe->objectForKey("weaponClass").toString(), QString("Axe"));

	// Case 3 - fake object will fail in the fetch
	PFObjectPtr fakeObject = PFObject::objectWithClassName("Faker", "s34af34a3f");
	QCOMPARE(fakeObject->isDataAvailable(), false);
	PFErrorPtr fakeObjectFetchError;
	QCOMPARE(fakeObject->fetchIfNeeded(fakeObjectFetchError), false);
	QCOMPARE(fakeObjectFetchError.isNull(), false);
	QCOMPARE(fakeObjectFetchError->errorCode(), kPFErrorObjectNotFound);
}

void TestPFObject::test_fetchIfNeededInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;
	QObject::connect(this, SIGNAL(fetchEnded()), &eventLoop, SLOT(quit()));

	// Case 1 - new object won't fetch
	PFObjectPtr newObject = PFObject::objectWithClassName("NiceAndShiny");
	QCOMPARE(newObject->isDataAvailable(), true);
	QCOMPARE(newObject->fetchIfNeededInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), false);
	QCOMPARE(newObject->isDataAvailable(), true);

	// Case 2 - server object fetch if needed
	PFObjectPtr axe = PFObject::objectWithClassName(_axe->className(), _axe->objectId());
	QCOMPARE(axe->isDataAvailable(), false);
	QCOMPARE(axe->fetchIfNeededInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, true);
	QCOMPARE(_fetchError.isNull(), true);
	_fetchSucceeded = false;
	_fetchError = PFErrorPtr();
	QCOMPARE(axe->isDataAvailable(), true);

	// Re-fetch will fail since the data is now available
	QCOMPARE(axe->fetchIfNeededInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), false);
	QCOMPARE(axe->objectForKey("weaponClass").toString(), QString("Axe"));

	// Case 3 - fake object will fail in the fetch
	PFObjectPtr fakeObject = PFObject::objectWithClassName("Faker", "s34af34a3f");
	QCOMPARE(fakeObject->isDataAvailable(), false);
	QCOMPARE(fakeObject->fetchIfNeededInBackground(this, SLOT(fetchCompleted(bool, PFErrorPtr))), true);
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_fetchSucceeded, false);
	QCOMPARE(_fetchError.isNull(), false);
	QCOMPARE(_fetchError->errorCode(), kPFErrorObjectNotFound);
}

void TestPFObject::test_fromJson()
{
	// Convert the sword object to json
	QJsonObject jsonSword;
	QCOMPARE(_sword->toJson(jsonSword), true);

	// Convert the json back to a sword
	QVariant swordVariant = PFObject::fromJson(jsonSword);
	PFObjectPtr convertedSword = PFObject::objectFromVariant(swordVariant);

	// Test the results
	QCOMPARE(convertedSword.isNull(), false);
	QCOMPARE(convertedSword->pfClassName(), _sword->pfClassName());
	QCOMPARE(convertedSword->objectId(), _sword->objectId());
}

void TestPFObject::test_toJson()
{
	// Invalid Case - Not uploaded yet
	QJsonObject invalidJsonObject;
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->toJson(invalidJsonObject), false);

	// Valid Case
	QJsonObject jsonObject;
	PFObjectPtr uploadedLevel = PFObject::objectWithClassName("Level", "39sdfji390j");
	QCOMPARE(uploadedLevel->toJson(jsonObject), true);
	QCOMPARE(jsonObject.contains("__type"), true);
	QCOMPARE(jsonObject.contains("className"), true);
	QCOMPARE(jsonObject.contains("objectId"), true);
	QCOMPARE(jsonObject["__type"].toString(), QString("Pointer"));
	QCOMPARE(jsonObject["className"].toString(), QString("Level"));
	QCOMPARE(jsonObject["objectId"].toString(), QString("39sdfji390j"));
}

void TestPFObject::test_pfClassName()
{
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->pfClassName(), QString("PFObject"));
}

void TestPFObject::test_savingWithPermissions()
{
	// Sign up a test user
	PFUserPtr user = PFUser::user();
	user->setUsername("TestPFObject-test_savingWithPermissions");
	user->setEmail("TestPFObject-test_savingWithPermissions@parse.com");
	user->setPassword("testPassword");
	QCOMPARE(PFUser::signUpWithUser(user), true);

	// Create an ACL that only allows access for that user
	PFACLPtr acl = PFACL::ACLWithUser(user);

	// Create an object and save it to the cloud
	PFObjectPtr sofa = PFObject::objectWithClassName("Furniture");
	sofa->setObjectForKey(QString("Sofa"), "furnitureType");
	sofa->setACL(acl);
	QCOMPARE(sofa->save(), true);

	// Let's log out
	PFUser::logOut();

	// Now modify the sofa and try to update it (should fail because we don't have write access)
	sofa->setObjectForKey(QString("LoveSeat"), "furnitureType");
	PFErrorPtr sofaSaveError;
	QCOMPARE(sofa->save(sofaSaveError), false);
	QCOMPARE(sofaSaveError->errorCode(), kPFErrorObjectNotFound);

	// Let's log back in and resave
	QCOMPARE(PFUser::logInWithUsernameAndPassword(user->username(), user->password()).isNull(), false);
	QCOMPARE(sofa->save(), true);

	// Cleanup the sofa and user
	QCOMPARE(sofa->deleteObject(), true);
	QCOMPARE(PFUser::currentUser()->deleteObject(), true);
}

void TestPFObject::test_fetchingWithPermissions()
{
	// Sign up a test user
	PFUserPtr user = PFUser::user();
	user->setUsername("TestPFObject-test_fetchingWithPermissions");
	user->setEmail("TestPFObject-test_fetchingWithPermissions@parse.com");
	user->setPassword("testPassword");
	QCOMPARE(PFUser::signUpWithUser(user), true);

	// Create an ACL that only allows access for that user
	PFACLPtr acl = PFACL::ACLWithUser(user);

	// Create an object and save it to the cloud
	PFObjectPtr sofa = PFObject::objectWithClassName("Furniture");
	sofa->setObjectForKey(QString("Sofa"), "furnitureType");
	sofa->setACL(acl);
	QCOMPARE(sofa->save(), true);

	// Try to fetch the sofa from the cloud (should succeed since we're logged in as user)
	PFObjectPtr cloudSofa = PFObject::objectWithClassName(sofa->className(), sofa->objectId());
	QCOMPARE(cloudSofa->fetch(), true);

	// Let's log out
	PFUser::logOut();

	// Try to fetch the cloud sofa again and this time it should fail b/c we're not authenticated
	PFObjectPtr cloudSofa2 = PFObject::objectWithClassName(sofa->className(), sofa->objectId());
	QCOMPARE(cloudSofa2->fetch(), false);

	// Let's try to delete the sofa and the user (both should fail b/c we're not logged in)
	QCOMPARE(sofa->deleteObject(), false);
	QCOMPARE(user->deleteObject(), false);

	// Let's log back in so we can cleanup
	QCOMPARE(PFUser::logInWithUsernameAndPassword(user->username(), user->password()).isNull(), false);

	// Cleanup
	QCOMPARE(sofa->deleteObject(), true);
	QCOMPARE(PFUser::currentUser()->deleteObject(), true);
}

DECLARE_TEST(TestPFObject)
#include "TestPFObject.moc"
