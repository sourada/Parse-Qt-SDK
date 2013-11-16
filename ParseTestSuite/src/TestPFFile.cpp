//
//  TestPFFile.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#include "PFFile.h"
#include "PFManager.h"
#include "TestRunner.h"

using namespace parse;

class TestPFFile : public QObject
{
    Q_OBJECT

public slots:

	void saveProgressUpdated(double percentDone)
	{
		Q_UNUSED(percentDone);
	}

	void saveCompleted(bool succeeded, PFErrorPtr error)
	{
		_saveSucceeded = succeeded;
		_saveError = error;
		emit saveEnded();
	}

	void getDataProgressUpdated(double percentDone)
	{
		Q_UNUSED(percentDone);
	}

	void getDataCompleted(QByteArray* data, PFErrorPtr error)
	{
		_getDataSucceeded = (data != NULL);
		_getDataError = error;
		emit getDataEnded();
	}

signals:

	void saveEnded();
	void getDataEnded();

private slots:

	//////////////////////////////////////////////////////////////////////
	//              Class init and cleanup methods
	//////////////////////////////////////////////////////////////////////

	void initTestCase()
	{
		// Build some default data
		_data = QByteArrayPtr(new QByteArray());
		_data->append(QString("Some sample data to test with").toUtf8());

		// Set the data path
		QDir currentDir = QDir::current();
		currentDir.cdUp();
		_dataPath = currentDir.absoluteFilePath("data");
	}

	void cleanupTestCase()
	{
		_data = QByteArrayPtr();
	}

	//////////////////////////////////////////////////////////////////////
	//   Test init and cleanup methods (called before/after each test)
	//////////////////////////////////////////////////////////////////////

	// Function init and cleanup methods (called before/after each test)
	void init()
	{
		_dataFile = PFFile::fileWithData(_data);
		_nameDataFile = PFFile::fileWithNameAndData("parse_file.txt", _data);

		QString filename = "plain_text.txt";
		QString plainTextFilepath = QDir(_dataPath).absoluteFilePath(filename);
		_nameContentsFile = PFFile::fileWithNameAndContentsAtPath("plain_text.txt", plainTextFilepath);

		QString name("3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt");
		QString url("http://files.parse.com/4744485e-5e69-4b8b-8e3e-01184261c172/3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt");
		_nameUrlFile = PFFile::fileWithNameAndUrl(name, url);

		// Make sure our cache is clean
		PFManager::sharedManager()->clearCache();

		// Set our save and get data flags to their defaults
		_saveSucceeded = false;
		_saveError = PFErrorPtr();
		_getDataSucceeded = false;
		_getDataError = PFErrorPtr();
	}

	void cleanup()
	{
		// Clear out each file
		_dataFile = PFFilePtr();
		_nameDataFile = PFFilePtr();
		_nameContentsFile = PFFilePtr();
		_nameUrlFile = PFFilePtr();
	}

	//////////////////////////////////////////////////////////////////////
	//                               Tests
	//////////////////////////////////////////////////////////////////////

	// Creation Methods for Upload
	void test_fileWithData();
	void test_fileWithNameAndData();
	void test_fileWithNameAndContentsAtPath();
	void test_fileFromVariant();
	void test_fileWithNameAndUrl();

	// Getter Methods
	void test_filepath();
	void test_name();
	void test_url();
	void test_isDirty();

	// Save Methods
	void test_save();
	void test_saveWithError();
	void test_saveInBackground();
	void test_saveInBackgroundWithProgress();

	// Get Data Methods
	void test_isDataAvailable();
	void test_getData();
	void test_getDataInBackground();
	void test_getDataInBackgroundWithProgress();

	// Cancellation Methods
	void test_cancel();

	// PFSerializable Methods
	void test_fromJson();
	void test_toJson();
	void test_pfClassName();

private:

	// Instance members
	QByteArrayPtr	_data;
	QString			_dataPath;
	PFFilePtr		_dataFile;
	PFFilePtr		_nameDataFile;
	PFFilePtr		_nameContentsFile;
	PFFilePtr		_nameUrlFile;

	// Instance members for save and get data callbacks
	bool			_saveSucceeded;
	PFErrorPtr		_saveError;
	bool			_getDataSucceeded;
	PFErrorPtr		_getDataError;
};

void TestPFFile::test_fileWithData()
{
	// Valid case with text data
	PFFilePtr file = PFFile::fileWithData(_data);
	QCOMPARE(file->getData(), _data.data());
	QCOMPARE(file->isDirty(), true);
	QCOMPARE(file->name(), QString("parse_file-no_name.txt"));
	QCOMPARE(file->filepath(), QString(""));
	QCOMPARE(file->url(), QString(""));

	// Invalid case where we send in an empty data pointer
	PFFilePtr emptyFile = PFFile::fileWithData(QByteArrayPtr());
	QCOMPARE(emptyFile, PFFilePtr());
}

void TestPFFile::test_fileWithNameAndData()
{
	// Invalid Case 1 - two bad inputs
	PFFilePtr invalidFile1 = PFFile::fileWithNameAndData("", QByteArrayPtr());
	QCOMPARE(invalidFile1, PFFilePtr());

	// Invalid Case 2 - bad name
	PFFilePtr invalidFile2 = PFFile::fileWithNameAndData("", _data);
	QCOMPARE(invalidFile2, PFFilePtr());

	// Invalid Case 3 - bad data
	PFFilePtr invalidFile3 = PFFile::fileWithNameAndData("parse_file.txt", QByteArrayPtr());
	QCOMPARE(invalidFile3, PFFilePtr());

	// Valid Case
	PFFilePtr file = PFFile::fileWithNameAndData("parse_file.txt", _data);
	QCOMPARE(file->getData(), _data.data());
	QCOMPARE(file->isDirty(), true);
	QCOMPARE(file->name(), QString("parse_file.txt"));
	QCOMPARE(file->filepath(), QString(""));
	QCOMPARE(file->url(), QString(""));
}

void TestPFFile::test_fileWithNameAndContentsAtPath()
{
	QString filename = "plain_text.txt";
	QString plainTextFilepath = QDir(_dataPath).absoluteFilePath(filename);

	// Invalid Case 1 - two bad inputs
	PFFilePtr invalidFile1 = PFFile::fileWithNameAndContentsAtPath("", "");
	QCOMPARE(invalidFile1, PFFilePtr());

	// Invalid Case 2 - bad name
	PFFilePtr invalidFile2 = PFFile::fileWithNameAndContentsAtPath("", plainTextFilepath);
	QCOMPARE(invalidFile2, PFFilePtr());

	// Invalid Case 3 - filepath that is a directory
	PFFilePtr invalidFile3 = PFFile::fileWithNameAndContentsAtPath(filename, _dataPath);
	QCOMPARE(invalidFile3, PFFilePtr());

	// Valid Case
	PFFilePtr file = PFFile::fileWithNameAndContentsAtPath(filename, plainTextFilepath);
	QCOMPARE(file->isDirty(), true);
	QCOMPARE(file->name(), filename);
	QCOMPARE(file->filepath(), plainTextFilepath);
	QCOMPARE(file->url(), QString(""));

	// Valid Case - data test
	QFile plainTextData(plainTextFilepath);
	plainTextData.open(QIODevice::ReadOnly);
	QByteArrayPtr expectedPlainTextData = QByteArrayPtr(new QByteArray());
	expectedPlainTextData->append(plainTextData.readAll());
	plainTextData.close();
	QCOMPARE(QString(*(file->getData())), QString(*(expectedPlainTextData.data())));
}

void TestPFFile::test_fileFromVariant()
{
	// Valid Case
	PFFilePtr file = PFFile::fileWithNameAndData("tutorialFile.txt", _data);
	QVariant fileVariant = PFFile::toVariant(file);
	PFFilePtr convertedFile = PFFile::fileFromVariant(fileVariant);
	QCOMPARE(convertedFile.isNull(), false);
	QCOMPARE(convertedFile->name(), QString("tutorialFile.txt"));

	// Invalid Case - QString
	QVariant stringVariant = QString("StringVariant");
	PFFilePtr convertedString = PFFile::fileFromVariant(stringVariant);
	QCOMPARE(convertedString.isNull(), true);
}

void TestPFFile::test_fileWithNameAndUrl()
{
	QString name("3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt");
	QString url("http://files.parse.com/4744485e-5e69-4b8b-8e3e-01184261c172/3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt");

	// Invalid Case 1 - two bad inputs
	PFFilePtr invalidFile1 = PFFile::fileWithNameAndUrl("", "");
	QCOMPARE(invalidFile1, PFFilePtr());

	// Invalid Case 2 - bad name
	PFFilePtr invalidFile2 = PFFile::fileWithNameAndUrl("", url);
	QCOMPARE(invalidFile2, PFFilePtr());

	// Invalid Case 3 - bad url
	PFFilePtr invalidFile3 = PFFile::fileWithNameAndUrl(name, "");
	QCOMPARE(invalidFile3, PFFilePtr());

	// Valid Case
	PFFilePtr file = PFFile::fileWithNameAndUrl(name, url);
	QCOMPARE(file->isDirty(), false);
	QCOMPARE(file->name(), name);
	QCOMPARE(file->filepath(), QString(""));
	QCOMPARE(file->url(), url);
	QCOMPARE(file->getData() == NULL, true);
}

void TestPFFile::test_filepath()
{
	QCOMPARE(_dataFile->filepath(), QString(""));
	QCOMPARE(_nameDataFile->filepath(), QString(""));
	QCOMPARE(_nameContentsFile->filepath(), QDir(_dataPath).absoluteFilePath("plain_text.txt"));
	QCOMPARE(_nameUrlFile->filepath(), QString(""));
}

void TestPFFile::test_name()
{
	QCOMPARE(_dataFile->name(), QString("parse_file-no_name.txt"));
	QCOMPARE(_nameDataFile->name(), QString("parse_file.txt"));
	QCOMPARE(_nameContentsFile->name(), QString("plain_text.txt"));
	QCOMPARE(_nameUrlFile->name(), QString("3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt"));
}

void TestPFFile::test_url()
{
	QCOMPARE(_dataFile->url(), QString(""));
	QCOMPARE(_nameDataFile->url(), QString(""));
	QCOMPARE(_nameContentsFile->url(), QString(""));
	QCOMPARE(_nameUrlFile->url(), QString("http://files.parse.com/4744485e-5e69-4b8b-8e3e-01184261c172/3864df34-fc29-45ff-9c2e-ef49fce53b45f-parse_plain_text.txt"));
}

void TestPFFile::test_isDirty()
{
	QCOMPARE(_dataFile->isDirty(), true);
	QCOMPARE(_nameDataFile->isDirty(), true);
	QCOMPARE(_nameContentsFile->isDirty(), true);
	QCOMPARE(_nameUrlFile->isDirty(), false);
}

void TestPFFile::test_save()
{
	QCOMPARE(_dataFile->save(), true);
	QCOMPARE(_dataFile->name().isEmpty(), false);
	QCOMPARE(_dataFile->url().isEmpty(), false);

	QCOMPARE(_nameDataFile->save(), true);
	QCOMPARE(_nameDataFile->name().isEmpty(), false);
	QCOMPARE(_nameDataFile->url().isEmpty(), false);

	QCOMPARE(_nameContentsFile->save(), true);
	QCOMPARE(_nameContentsFile->name().isEmpty(), false);
	QCOMPARE(_nameContentsFile->url().isEmpty(), false);

	// Should fail b/c created with the fileWithNameAndUrl method (hints that it has already been saved)
	QCOMPARE(_nameUrlFile->save(), false);
}

void TestPFFile::test_saveWithError()
{
	PFErrorPtr error;
	QCOMPARE(_dataFile->save(error), true);
	QCOMPARE(error, PFErrorPtr());
	QCOMPARE(_dataFile->name().isEmpty(), false);
	QCOMPARE(_dataFile->url().isEmpty(), false);

	error = PFErrorPtr();
	QCOMPARE(_nameDataFile->save(error), true);
	QCOMPARE(error, PFErrorPtr());
	QCOMPARE(_nameDataFile->name().isEmpty(), false);
	QCOMPARE(_nameDataFile->url().isEmpty(), false);

	error = PFErrorPtr();
	QCOMPARE(_nameContentsFile->save(error), true);
	QCOMPARE(error, PFErrorPtr());
	QCOMPARE(_nameContentsFile->name().isEmpty(), false);
	QCOMPARE(_nameContentsFile->url().isEmpty(), false);

	// Should fail b/c created with the fileWithNameAndUrl method (hints that it has already been saved)
	error = PFErrorPtr();
	QCOMPARE(_nameUrlFile->save(error), false);
	QCOMPARE(error, PFErrorPtr());
}

void TestPFFile::test_saveInBackground()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Data File Save
	_dataFile->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/Data File Save
	_saveSucceeded = false;
	_saveError = PFErrorPtr();
	_nameDataFile->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/Contents File Save
	_saveSucceeded = false;
	_saveError = PFErrorPtr();
	_nameContentsFile->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/URL File Save
	// NOTE: Should fail b/c created with the fileWithNameAndUrl method (hints that it has already been saved)
	bool succeeded = _nameUrlFile->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QCOMPARE(succeeded, false);
}

void TestPFFile::test_saveInBackgroundWithProgress()
{
	// Use an event loop to block until we receive the completion
	QEventLoop eventLoop;

	// Data File Save
	_dataFile->saveInBackground(this, SLOT(saveProgressUpdated(double)), this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QObject::connect(this, SIGNAL(saveEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/Data File Save
	_saveSucceeded = false;
	_saveError = PFErrorPtr();
	_nameDataFile->saveInBackground(this, SLOT(saveProgressUpdated(double)), this, SLOT(saveCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/Contents File Save
	_saveSucceeded = false;
	_saveError = PFErrorPtr();
	_nameContentsFile->saveInBackground(this, SLOT(saveProgressUpdated(double)), this, SLOT(saveCompleted(bool, PFErrorPtr)));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(_saveSucceeded, true);
	QCOMPARE(_saveError, PFErrorPtr());

	// Name/URL File Save
	// NOTE: Should fail b/c created with the fileWithNameAndUrl method (hints that it has already been saved)
	bool succeeded = _nameUrlFile->saveInBackground(this, SLOT(saveProgressUpdated(double)), this, SLOT(saveCompleted(bool, PFErrorPtr)));
	QCOMPARE(succeeded, false);
}

void TestPFFile::test_isDataAvailable()
{
	// Check the in-memory case
	QCOMPARE(_dataFile->isDataAvailable(), true);
	QCOMPARE(_nameDataFile->isDataAvailable(), true);
	QCOMPARE(_nameContentsFile->isDataAvailable(), true);
	QCOMPARE(_nameUrlFile->isDataAvailable(), false);

	// Put the data file in the cloud
	bool saved = _dataFile->save();
	QCOMPARE(saved, true);

	// Create a new file that matches the data file name and url and get the data which will pull
	// the data into memory and into the cache
	QEventLoop eventLoop;
	PFFilePtr cloudFile = PFFile::fileWithNameAndUrl(_dataFile->name(), _dataFile->url());
	QCOMPARE(cloudFile->isDataAvailable(), false);
	bool getDataOperationStarted = cloudFile->getDataInBackground(this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getDataOperationStarted, true);
	QObject::connect(this, SIGNAL(getDataEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(cloudFile->isDataAvailable(), true);

	// Create a new file matching the cloud file to test the in-cache case since it won't be in memory
	PFFilePtr cacheFile = PFFile::fileWithNameAndUrl(cloudFile->name(), cloudFile->url());
	QCOMPARE(cacheFile->isDataAvailable(), true);
}

void TestPFFile::test_getData()
{
	// Check the in-memory case
	QCOMPARE(_dataFile->getData() != NULL, true);
	QCOMPARE(_nameDataFile->getData() != NULL, true);
	QCOMPARE(_nameContentsFile->getData() != NULL, true);
	QCOMPARE(_nameUrlFile->getData() != NULL, false);

	// Put the data file in the cloud
	bool saved = _dataFile->save();
	QCOMPARE(saved, true);

	// Create a new file that matches the data file name and url and get the data which will pull
	// the data into memory and into the cache
	QEventLoop eventLoop;
	PFFilePtr cloudFile = PFFile::fileWithNameAndUrl(_dataFile->name(), _dataFile->url());
	QCOMPARE(cloudFile->getData() != NULL, false);
	bool getDataOperationStarted = cloudFile->getDataInBackground(this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getDataOperationStarted, true);
	QObject::connect(this, SIGNAL(getDataEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(cloudFile->getData() != NULL, true);

	// Create a new file matching the cloud file to test the in-cache case since it won't be in memory
	PFFilePtr cacheFile = PFFile::fileWithNameAndUrl(cloudFile->name(), cloudFile->url());
	QCOMPARE(cacheFile->getData() != NULL, true);
}

void TestPFFile::test_getDataInBackground()
{
	// Test the files where the data is already available
	QCOMPARE(_dataFile->getDataInBackground(NULL, ""), false);
	QCOMPARE(_nameDataFile->getDataInBackground(NULL, ""), false);
	QCOMPARE(_nameContentsFile->getDataInBackground(NULL, ""), false);

	// Put the test zip file in the cloud
	QString filename = "archive_file.zip";
	QString filepath = QDir(_dataPath).absoluteFilePath(filename);
	PFFilePtr zipFile = PFFile::fileWithNameAndContentsAtPath(filename, filepath);
	QCOMPARE(zipFile->getData() != NULL, true);
	QCOMPARE(zipFile->isDirty(), true);
	bool saved = zipFile->save();
	QCOMPARE(saved, true);
	QCOMPARE(zipFile->getData() != NULL, true);

	// Create a new file to pull it back out of the cloud
	PFFilePtr cloudFile = PFFile::fileWithNameAndUrl(zipFile->name(), zipFile->url());
	QCOMPARE(cloudFile->getData() != NULL, false);
	QEventLoop eventLoop;
	bool getDataOperationStarted = cloudFile->getDataInBackground(this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getDataOperationStarted, true);
	QObject::connect(this, SIGNAL(getDataEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(cloudFile->getData() != NULL, true);
}

void TestPFFile::test_getDataInBackgroundWithProgress()
{
	// Test the files where the data is already available
	QCOMPARE(_dataFile->getDataInBackground(NULL, "", NULL, ""), false);
	QCOMPARE(_nameDataFile->getDataInBackground(NULL, "", NULL, ""), false);
	QCOMPARE(_nameContentsFile->getDataInBackground(NULL, "", NULL, ""), false);

	// Put the test image file in the cloud
	QString filename = "small_image.jpg";
	QString filepath = QDir(_dataPath).absoluteFilePath(filename);
	PFFilePtr imageFile = PFFile::fileWithNameAndContentsAtPath(filename, filepath);
	QCOMPARE(imageFile->getData() != NULL, true);
	QCOMPARE(imageFile->isDirty(), true);
	bool saved = imageFile->save();
	QCOMPARE(saved, true);
	QCOMPARE(imageFile->getData() != NULL, true);

	// Create a new file to pull the image data back out of the cloud
	PFFilePtr cloudFile = PFFile::fileWithNameAndUrl(imageFile->name(), imageFile->url());
	QCOMPARE(cloudFile->getData() != NULL, false);
	QEventLoop eventLoop;
	bool getDataOperationStarted = cloudFile->getDataInBackground(this, SLOT(getDataProgressUpdated(double)), this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getDataOperationStarted, true);
	QObject::connect(this, SIGNAL(getDataEnded()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
	QCOMPARE(cloudFile->getData() != NULL, true);
}

void TestPFFile::test_cancel()
{
	// Cancel a file that isn't uploading (should just return)
	_nameDataFile->cancel();

	// Cancel a file that isn't downloading (should just return)
	_nameUrlFile->cancel();

	// Start a save in background operation and cancel it, then try to save it again and it should succeed. If it
	// wasn't cancelled properly, the second save would fail b/c the data would already have been saved.
	_nameDataFile->saveInBackground(this, SLOT(saveCompleted(bool, PFErrorPtr)));
	_nameDataFile->cancel();
	QCOMPARE(_nameDataFile->save(), true);

	// Create a cloud file from the name/data file and cancel the get data in background method. Then retry to get
	// the get in the background and it should succeed.
	PFFilePtr cloudFile = PFFile::fileWithNameAndUrl(_nameDataFile->name(), _nameDataFile->url());
	bool getStarted = cloudFile->getDataInBackground(this, SLOT(getDataProgressUpdated(double)), this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getStarted, true);
	cloudFile->cancel();
	bool getStartedAgain = cloudFile->getDataInBackground(this, SLOT(getDataProgressUpdated(double)), this, SLOT(getDataCompleted(QByteArray*, PFErrorPtr)));
	QCOMPARE(getStartedAgain, true);
	cloudFile->cancel();
}

void TestPFFile::test_fromJson()
{
	// Convert the name url file to json
	QJsonObject jsonObject;
	QCOMPARE(_nameUrlFile->toJson(jsonObject), true);
	jsonObject["url"] = _nameUrlFile->url();

	// Convert the json back to a file
	QVariant fileVariant = PFFile::fromJson(jsonObject);
	PFFilePtr convertedFile = PFFile::fileFromVariant(fileVariant);

	// Test the results of the conversion
	QCOMPARE(convertedFile.isNull(), false);
	QCOMPARE(convertedFile->name(), _nameUrlFile->name());
	QCOMPARE(convertedFile->url(), _nameUrlFile->url());
}

void TestPFFile::test_toJson()
{
	// Try to serialize all the upload files that will fail since they haven't been uploaded yet
	QJsonObject jsonObject;
	QCOMPARE(_dataFile->toJson(jsonObject), false);
	QCOMPARE(_nameDataFile->toJson(jsonObject), false);
	QCOMPARE(_nameContentsFile->toJson(jsonObject), false);

	// Test a valid case
	QCOMPARE(_nameUrlFile->toJson(jsonObject), true);
	QCOMPARE(jsonObject.contains("__type"), true);
	QCOMPARE(jsonObject.contains("name"), true);
	QCOMPARE(jsonObject["__type"].toString(), QString("File"));
	QCOMPARE(jsonObject["name"].toString(), _nameUrlFile->name());
}

void TestPFFile::test_pfClassName()
{
	QCOMPARE(_dataFile->pfClassName(), QString("PFFile"));
	QCOMPARE(_nameDataFile->pfClassName(), QString("PFFile"));
	QCOMPARE(_nameContentsFile->pfClassName(), QString("PFFile"));
	QCOMPARE(_nameUrlFile->pfClassName(), QString("PFFile"));
}

DECLARE_TEST(TestPFFile)
#include "TestPFFile.moc"
