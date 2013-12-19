//
//  PFFile.cpp
//  Parse
//
//  Created by Christian Noon on 11/6/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFError.h"
#include "PFFile.h"
#include "PFManager.h"

// Qt headers
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QNetworkRequest>
#include <QUrl>

namespace parse {

// Static Globals
static QString gDefaultName = "parse_file-no_name";

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFFile::PFFile()
{
	qDebug().nospace() << "Created PFFile(" << QString().sprintf("%8p", this) << ")";

	_filepath = "";
	_mimeType = "";
	_name = "";
	_url = "";
	_data = QByteArrayPtr();
	_tempDownloadData = QByteArrayPtr();
	_isDirty = false;
	_isUploading = false;
	_isDownloading = false;
	_saveReply = NULL;
	_getDataReply = NULL;
}

PFFile::~PFFile()
{
	qDebug().nospace() << "Destroyed PFFile(" << QString().sprintf("%8p", this) << ")";
}

#ifdef __APPLE__
#pragma mark - Creation Methods for Upload
#endif

PFFilePtr PFFile::fileWithData(QByteArrayPtr data)
{
	// Make sure the parameters are valid
	if (data.isNull())
	{
		qWarning() << "PFFile::fileWithData failed because the set up parameters were not valid";
		return PFFilePtr();
	}
	else
	{
		// Create a new file
		PFFilePtr file = PFFilePtr(new PFFile(), &QObject::deleteLater);

		// Mark the file as dirty because it needs to be uploaded
		file->_isDirty = true;

		// Grab the mime type
		QMimeDatabase mimeDatabase;
		QMimeType mimeType = mimeDatabase.mimeTypeForData(*(data.data()));
		file->_mimeType = mimeType.filterString();

		// Set the name to upload
		file->_name = gDefaultName;
		if (!mimeType.preferredSuffix().isEmpty())
			file->_name += "." + mimeType.preferredSuffix();

		// Store a shared reference to the data
		file->_data = data;

		return file;
	}
}

PFFilePtr PFFile::fileWithNameAndData(const QString& name, QByteArrayPtr data)
{
	// Make sure the parameters are valid
	if (name.isEmpty() || data.isNull())
	{
		qWarning() << "PFFile::fileWithNameAndData failed because the set up parameters were not valid";
		return PFFilePtr();
	}
	else
	{
		// Create a new file
		PFFilePtr file = PFFilePtr(new PFFile(), &QObject::deleteLater);

		// Mark the file as dirty because it needs to be uploaded
		file->_isDirty = true;

		// Grab the mime type
		QMimeDatabase mimeDatabase;
		QMimeType mimeType = mimeDatabase.mimeTypeForData(*(data.data()));
		file->_mimeType = mimeType.filterString();

		// Set the name to upload
		file->_name = name;

		// Store a shared reference to the data
		file->_data = data;

		return file;
	}
}

PFFilePtr PFFile::fileWithNameAndContentsAtPath(const QString& name, const QString& filepath)
{
	// Make sure the parameters are valid
	QFileInfo fileInfo(filepath);
	if (!fileInfo.isFile() || name.isEmpty())
	{
		qWarning() << "PFFile::fileWithNameAndContentsAtPath failed because the set up parameters were not valid";
		return PFFilePtr();
	}
	else
	{
		// Create a new file
		PFFilePtr file = PFFilePtr(new PFFile(), &QObject::deleteLater);

		// Mark the file as dirty because it needs to be uploaded
		file->_isDirty = true;

		// Grab the mime type
		QMimeDatabase mimeDatabase;
		QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileInfo);
		file->_mimeType = mimeType.filterString();

		// Store the name and filepath
		file->_name = name;
		file->_filepath = filepath;

		// Load the data out of the file and store it in data
		QFile fileData(filepath);
		fileData.open(QIODevice::ReadOnly);
		file->_data = QByteArrayPtr(new QByteArray());
		file->_data->append(fileData.readAll());
		fileData.close();

		return file;
	}
}

PFFilePtr PFFile::fileFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = variant.value<PFSerializablePtr>();
	if (!serializable.isNull())
		return serializable.objectCast<PFFile>();

	return PFFilePtr();
}

#ifdef __APPLE__
#pragma mark - Creation Methods for Download
#endif

PFFilePtr PFFile::fileWithNameAndUrl(const QString& name, const QString& url)
{
	// Make sure the parameters are valid
	if (name.isEmpty() || url.isEmpty())
	{
		qWarning() << "FAILURE: PFFile::fileWithNameAndUrl() failed because the set up parameters were not valid";
		return PFFilePtr();
	}
	else
	{
		// Create a new file
		PFFilePtr file = PFFilePtr(new PFFile(), &QObject::deleteLater);
		file->_name = name;
		file->_url = url;

		return file;
	}
}

#ifdef __APPLE__
#pragma mark - Object Info Getter Methods
#endif

const QString& PFFile::filepath()
{
	return _filepath;
}

const QString& PFFile::name()
{
	return _name;
}

const QString& PFFile::url()
{
	return _url;
}

bool PFFile::isDirty()
{
	return _isDirty;
}

#ifdef __APPLE__
#pragma mark - Save Methods
#endif

bool PFFile::save()
{
	PFErrorPtr error;
	return save(error);
}

bool PFFile::save(PFErrorPtr& error)
{
	// Early out if the file has already been uploaded
	if (!_isDirty)
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _name << "\" has already been saved";
		return false;
	}

	// Early out if the file is already uploading
	if (_isUploading)
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _filepath << "\" is already uploading";
		return false;
	}

	// Update the ivar
	_isUploading = true;

	// Create a network request
	QNetworkRequest request = createSaveNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	QNetworkReply* networkReply = networkAccessManager->post(request, *(_data.data()));

	// Block the async nature of the request using our own event loop until the reply finishes
	QEventLoop eventLoop;
	QObject::connect(networkReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

	// Update our ivar
	_isUploading = false;

	// Deserialize the reply
	bool success = deserializeSaveNetworkReply(networkReply, error);

	// Remove the dirty flag if the upload succeeded
	if (success)
		_isDirty = false;

	// Clean up
	networkReply->deleteLater();

	return success;
}

bool PFFile::saveInBackground(QObject *target, const char *action)
{
	return saveInBackground(0, 0, target, action);
}

bool PFFile::saveInBackground(QObject *saveProgressTarget, const char *saveProgressAction, QObject *saveCompleteTarget, const char *saveCompleteAction)
{
	// Early out if the file has already been uploaded
	if (!_isDirty)
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _name << "\" has already been saved";
		return false;
	}

	// Early out if the file is already uploading
	if (_isUploading)
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _filepath << "\" is already uploading";
		return false;
	}

	// Update the ivar
	_isUploading = true;

	// Create a network request
	QNetworkRequest request = createSaveNetworkRequest();

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_saveReply = networkAccessManager->post(request, *(_data.data()));
	QObject::connect(_saveReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleSaveProgressUpdated(qint64, qint64)));
	QObject::connect(_saveReply, SIGNAL(finished()), this, SLOT(handleSaveCompleted()));

	// Connect the callbacks from this object to the target actions
	if (saveProgressTarget)
		QObject::connect(this, SIGNAL(saveProgressUpdated(double)), saveProgressTarget, saveProgressAction);
	if (saveCompleteTarget)
		QObject::connect(this, SIGNAL(saveCompleted(bool, PFErrorPtr)), saveCompleteTarget, saveCompleteAction);

	return true;
}

#ifdef __APPLE__
#pragma mark - Get Data Methods
#endif

bool PFFile::isDataAvailable()
{
	bool isInMemory = !_data.isNull();
	bool isInCache = QFileInfo(PFManager::sharedManager()->cacheDirectory().filePath(_name)).isFile();
	return (isInMemory || isInCache);
}

QByteArray* PFFile::getData()
{
	// Use the data in memory if it exists
	if (!_data.isNull())
		return _data.data();

	// If the data is available, then we need to load it out of the cache
	if (isDataAvailable())
	{
		QString filepath = PFManager::sharedManager()->cacheDirectory().filePath(_name);
		QFile file(filepath);
		file.open(QIODevice::ReadOnly);
		_data = QByteArrayPtr(new QByteArray());
		_data->append(file.readAll());
		file.close();
		return _data.data();
	}

	return NULL;
}

bool PFFile::getDataInBackground(QObject *target, const char *action)
{
	return getDataInBackground(0, 0, target, action);
}

bool PFFile::getDataInBackground(QObject *getDataProgressTarget, const char *getDataProgressAction,
								 QObject *getDataCompleteTarget, const char *getDataCompleteAction)
{
	// Early out if the file is already downloading
	if (_isDownloading)
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _name << "\" is already downloading";
		return false;
	}

	// Early out if the file has already been downloaded
	if (isDataAvailable())
	{
		qWarning().nospace() << "WARNING: PFFile \"" << _name << "\" is already downloaded, use PFFile::getData() method instead";
		return false;
	}

	// Update the ivar
	_isDownloading = true;

	// Create a new temp data object to sequentially write the data into
	_tempDownloadData = QByteArrayPtr(new QByteArray());

	// Create a network request
	QUrl url = QUrl(_url);
	QNetworkRequest request(url);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	_getDataReply = networkAccessManager->get(request);
	QObject::connect(_getDataReply, SIGNAL(readyRead()), this, SLOT(handleGetDataReadyRead()));
	QObject::connect(_getDataReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleGetDataProgressUpdated(qint64, qint64)));
	QObject::connect(_getDataReply, SIGNAL(finished()), this, SLOT(handleGetDataCompleted()));

	// Connect the callbacks from this object to the target actions
	if (getDataProgressTarget)
		QObject::connect(this, SIGNAL(getDataProgressUpdated(double)), getDataProgressTarget, getDataProgressAction);
	if (getDataCompleteTarget)
		QObject::connect(this, SIGNAL(getDataCompleted(QByteArray*, PFErrorPtr)), getDataCompleteTarget, getDataCompleteAction);

	return true;
}

#ifdef __APPLE__
#pragma mark - Cancellation Methods
#endif

void PFFile::cancel()
{
	// Cancel upload if active
	if (_isUploading)
	{
		qDebug() << "Cancelling save operation";
		disconnect(SIGNAL(saveProgressUpdated(double)));
		disconnect(SIGNAL(saveCompleted(bool, PFErrorPtr)));
		_saveReply->disconnect();
		_saveReply->abort();
		_saveReply->deleteLater();
		_isUploading = false;
	}

	// Cancel download if active
	if (_isDownloading)
	{
		qDebug() << "Cancelling get data operation";
		disconnect(SIGNAL(getDataProgressUpdated(double)));
		disconnect(SIGNAL(getDataCompleted(QByteArray*, PFErrorPtr)));
		_getDataReply->disconnect();
		_getDataReply->abort();
		_getDataReply->deleteLater();
		_isDownloading = false;
	}
}

#ifdef __APPLE__
#pragma mark - Backend API - PFSerializable Methods
#endif

QVariant PFFile::fromJson(const QJsonObject& jsonObject)
{
	QString name = jsonObject["name"].toString();
	QString url = jsonObject["url"].toString();
	PFFilePtr file = PFFile::fileWithNameAndUrl(name, url);

	return toVariant(file);
}

bool PFFile::toJson(QJsonObject& jsonObject)
{
	if (isDirty())
	{
		qWarning() << "PFFile::toJson could NOT convert to PFFile to JSON because the file is dirty and needs to be uploaded";
		return false;
	}
	else
	{
		jsonObject["__type"] = QString("File");
		jsonObject["name"] = _name;
		jsonObject["url"] = _url;
		return true;
	}
}

const QString PFFile::pfClassName() const
{
	return "PFFile";
}

#ifdef __APPLE__
#pragma mark - Protected Save Slots
#endif

void PFFile::handleSaveProgressUpdated(qint64 bytesSent, qint64 bytesTotal)
{
	double percentDone = 100.0;
	if (bytesSent != bytesTotal)
		percentDone = ((double) bytesSent / bytesTotal) * 100.0;
	emit saveProgressUpdated(percentDone);
}

void PFFile::handleSaveCompleted()
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);
	this->disconnect(SIGNAL(saveProgressUpdated(double)));

	// Update our ivar
	_isUploading = false;

	// Deserialize the reply
	PFErrorPtr error;
	bool success = deserializeSaveNetworkReply(_saveReply, error);

	// Remove the dirty flag if the upload succeeded
	if (success)
		_isDirty = false;

	// Emit the signal that the save has completed and then disconnect it
	emit saveCompleted(success, error);
	this->disconnect(SIGNAL(saveCompleted(bool, PFErrorPtr)));

	// Clean up
	_saveReply->deleteLater();
}

#ifdef __APPLE__
#pragma mark - Protected Get Data Slots
#endif

void PFFile::handleGetDataReadyRead()
{
	// Append the data
	_tempDownloadData->append(_getDataReply->readAll());
}

void PFFile::handleGetDataProgressUpdated(qint64 bytesSent, qint64 bytesTotal)
{
	double percentDone = 100.0;
	if (bytesSent != bytesTotal)
		percentDone = ((double) bytesSent / bytesTotal) * 100.0;
	emit getDataProgressUpdated(percentDone);
}

void PFFile::handleGetDataCompleted()
{
	// Disconnect the network access manager as well as all the connected signals to this instance
	QNetworkAccessManager* networkAccessManager = PFManager::sharedManager()->networkAccessManager();
	networkAccessManager->disconnect(this);
	this->disconnect(SIGNAL(getDataProgressUpdated(double)));

	// Update our ivar
	_isDownloading = false;

	// Extract the JSON payload
	if (_getDataReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Move the data from the temp download data object to the final data object
		_data = _tempDownloadData;
		_tempDownloadData.clear();

		// Write the data out to the cache
		QString filepath = PFManager::sharedManager()->cacheDirectory().filePath(_name);
		QFile file(filepath);
		file.open(QIODevice::WriteOnly);
		file.write(*(_data.data()));
		file.close();

		emit getDataCompleted(_data.data(), PFErrorPtr());
	}
	else // FAILURE
	{
		int errorCode = kPFErrorFileDownloadConnectionFailed;
		QString errorMessage = "File download connection failed";
		emit getDataCompleted(NULL, PFError::errorWithCodeAndMessage(errorCode, errorMessage));
	}

	// Disconnect the save completed signal
	this->disconnect(SIGNAL(getDataCompleted(QByteArray*, PFErrorPtr)));

	// Clean up
	_getDataReply->deleteLater();
}

#ifdef __APPLE__
#pragma mark - Network Request Builder Methods
#endif

QNetworkRequest PFFile::createSaveNetworkRequest()
{
	QUrl url = QUrl(QString("https://api.parse.com/1/files/") + _name);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::sharedManager()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::sharedManager()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), _mimeType.toUtf8());

	return request;
}

#ifdef __APPLE__
#pragma mark - Network Reply Deserialization Methods
#endif

bool PFFile::deserializeSaveNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error)
{
	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(networkReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Extract the JSON payload
	if (networkReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		_url = jsonObject["url"].toString();
		_name = jsonObject["name"].toString();

		return true;
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		error = PFError::errorWithCodeAndMessage(errorCode, errorMessage);

		return false;
	}
}

}	// End of parse namespace
