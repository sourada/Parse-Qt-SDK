//
//  TestPFObject.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
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

signals:

	void saveEnded();
	void deleteObjectEnded();

private slots:

	// Class init and cleanup methods
	void initTestCase()
	{
		PFUser::logOut();

		// Build some default data for a file
		_data = QByteArrayPtr(new QByteArray());
		_data->append(QString("Some sample data to test with").toUtf8());

		// Set the data path
		QDir currentDir = QDir::current();
		currentDir.cdUp();
		_dataPath = currentDir.absoluteFilePath("data");

		// Reset the callback flags
		_saveSucceeded = false;
		_saveError = PFErrorPtr();
		_deleteObjectSucceeded = false;
		_deleteObjectError = PFErrorPtr();
	}

	void cleanupTestCase()
	{
		// No-op
	}

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		// Reset the default ACL
		PFACL::setDefaultACLWithAccessForCurrentUser(PFACLPtr(), false);
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
	void test_objectForKey();
	void test_allKeys();

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

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_className();

private:

	// Instance members
	QString			_dataPath;
	QByteArrayPtr	_data;
	bool			_saveSucceeded;
	PFErrorPtr		_saveError;
	bool			_deleteObjectSucceeded;
	PFErrorPtr		_deleteObjectError;
};

void TestPFObject::test_objectWithClassName()
{
	// Invalid Object - Empty Class Name
	PFObjectPtr invalidObject = PFObject::objectWithClassName("");
	QCOMPARE(invalidObject.isNull(), true);

	// Valid Object - Level
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level.isNull(), false);
	QCOMPARE(level->parseClassName(), QString("Level"));
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
	QCOMPARE(level->parseClassName(), QString("Level"));
	QCOMPARE(level->objectId(), QString("309di3j09f"));
	QCOMPARE(level->ACL().isNull(), true);
	QCOMPARE(level->createdAt(), PFDateTimePtr());
	QCOMPARE(level->updatedAt(), PFDateTimePtr());
}

void TestPFObject::test_objectFromVariant()
{
	// Valid Case
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QVariant levelVariant = PFSerializable::toVariant(level);
	PFObjectPtr convertedLevel = PFObject::objectFromVariant(levelVariant);
	QCOMPARE(convertedLevel.isNull(), false);
	QCOMPARE(convertedLevel->parseClassName(), QString("Level"));

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFObjectPtr convertedString = PFObject::objectFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);

	// Invalid Case - PFDateTime
	PFDateTimePtr dateTime = PFDateTime::dateTimeFromDateTime(QDateTime::currentDateTime());
	QVariant dateTimeVariant = PFSerializable::toVariant(dateTime);
	PFObjectPtr dateTimeObject = PFObject::objectFromVariant(dateTimeVariant);
	QCOMPARE(dateTimeObject.isNull(), true);

	// Invalid Case - PFFile
	PFFilePtr file = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	QVariant fileVariant = PFSerializable::toVariant(file);
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
	towns << PFSerializable::toVariant(town1) << PFSerializable::toVariant(town2);
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
	level->setACL(PFACLPtr());
	QCOMPARE(level->ACL().isNull(), true);

	// TODO: Needs to also remove the ACL from the object
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
	level->setACL(PFACLPtr());
	QCOMPARE(level->ACL().isNull(), true);
}

void TestPFObject::test_parseClassName()
{
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->parseClassName(), QString("Level"));
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
	QCOMPARE(level->createdAt()->dateTime().date(), QDate::currentDate());

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
	QCOMPARE(level->updatedAt()->dateTime().date(), QDate::currentDate());

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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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
	characters << PFSerializable::toVariant(diablo) << PFSerializable::toVariant(wizard);
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

void TestPFObject::test_fromJson()
{
	// TODO: Not implemented yet
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

void TestPFObject::test_className()
{
	PFObjectPtr level = PFObject::objectWithClassName("Level");
	QCOMPARE(level->className(), QString("PFObject"));
}

DECLARE_TEST(TestPFObject)
#include "TestPFObject.moc"
