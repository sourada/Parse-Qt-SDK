//
//  PFFile.cpp
//  Parse
//
//  Created by Christian Noon on 11/6/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Qt headers
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

// Parse headers
#include "PFFile.h"
#include "PFManager.h"

namespace parse {

#pragma mark - Memory Management Methods

PFFile::PFFile(const QString& filepath)
{
	// Set the ivars
	QFile file(filepath);
	file.open(QIODevice::ReadOnly);
	_data = QSharedPointer<QByteArray>(new QByteArray());
	_data->append(file.readAll());
	file.close();
	_name = "";
	_url = "";
	_isDirty = true;
	_isUploading = false;
	_isDownloading = false;
	_saveReply = NULL;
	_getDataReply = NULL;

	// Make sure the mime type is supported
	QFileInfo fileInfo(filepath);
	QString extension = fileInfo.completeSuffix();
	_mimeType = convertExtensionToMimeType(extension);
	Q_ASSERT_X(!mimeType.isEmpty(), "PFFile upload constructor", "mime type NOT supported for extension");
}

PFFile::PFFile(const QString& name, const QString& url) :
	_filepath(""),
	_mimeType(""),
	_name(name),
	_url(url),
	_data(),
	_tempDownloadData(),
	_isDirty(false),
	_isUploading(false),
	_isDownloading(false),
	_saveReply(NULL),
	_getDataReply(NULL)
{
	// No-op
}

PFFile::~PFFile()
{
	// No-op
}

#pragma mark - Public User API Methods

bool PFFile::isDirty()
{
	return _isDirty;
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
	QFileInfo fileInfo(_filepath);
	QString filename = fileInfo.fileName();
	QUrl url = QUrl(QString("https://api.parse.com/1/files/") + filename);
	QNetworkRequest request(url);
	request.setRawHeader(QString("X-Parse-Application-Id").toUtf8(), PFManager::instance()->applicationId().toUtf8());
	request.setRawHeader(QString("X-Parse-REST-API-Key").toUtf8(), PFManager::instance()->restApiKey().toUtf8());
	request.setRawHeader(QString("Content-Type").toUtf8(), _mimeType.toUtf8());

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	_saveReply = networkAccessManager->post(request, *(_data.data()));
	QObject::connect(_saveReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleSaveProgressUpdated(qint64, qint64)));
	QObject::connect(_saveReply, SIGNAL(finished()), this, SLOT(handleSaveCompleted()));

	// Connect the callbacks from this object to the target actions
	QObject::connect(this, SIGNAL(saveProgressUpdated(double)), saveProgressTarget, saveProgressAction);
	QObject::connect(this, SIGNAL(saveCompleted(bool, PFErrorPtr)), saveCompleteTarget, saveCompleteAction);

	return true;
}

bool PFFile::isDataAvailable()
{
	bool isInMemory = !_data.isNull();
	bool isInCache = QFileInfo(PFManager::instance()->cacheDirectory().filePath(_name)).isFile();
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
		QString filepath = PFManager::instance()->cacheDirectory().filePath(_name);
		QFile file(filepath);
		file.open(QIODevice::ReadOnly);
		_data = QSharedPointer<QByteArray>(new QByteArray());
		_data->append(file.readAll());
		file.close();
		return _data.data();
	}

	return NULL;
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
		qWarning().nospace() << "WARNING: PFFile \"" << _name << "\" is already downloaded";
		return false;
	}

	// Update the ivar
	_isDownloading = true;

	// Create a new temp data object to sequentially write the data into
	_tempDownloadData = QSharedPointer<QByteArray>(new QByteArray());

	// Create a network request
	QUrl url = QUrl(_url);
	QNetworkRequest request(url);

	// Execute the request and connect the callbacks
	QNetworkAccessManager* networkAccessManager = PFManager::instance()->networkAccessManager();
	_getDataReply = networkAccessManager->get(request);
	QObject::connect(_getDataReply, SIGNAL(readyRead()), this, SLOT(handleGetDataReadyRead()));
	QObject::connect(_getDataReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleGetDataProgressUpdated(qint64, qint64)));
	QObject::connect(_getDataReply, SIGNAL(finished()), this, SLOT(handleGetDataCompleted()));

	// Connect the callbacks from this object to the target actions
	QObject::connect(this, SIGNAL(getDataProgressUpdated(double)), getDataProgressTarget, getDataProgressAction);
	QObject::connect(this, SIGNAL(getDataCompleted(QByteArray*, PFErrorPtr)), getDataCompleteTarget, getDataCompleteAction);

	return true;
}

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

#pragma mark - Protected Save Slots

void PFFile::handleSaveProgressUpdated(qint64 bytesSent, qint64 bytesTotal)
{
	double percentDone = 100.0;
	if (bytesSent != bytesTotal)
		percentDone = ((double) bytesSent / bytesTotal) * 100.0;
	emit saveProgressUpdated(percentDone);
}

void PFFile::handleSaveCompleted()
{
	// Update our ivar
	_isUploading = false;

	// Parse the json reply
	QJsonDocument doc = QJsonDocument::fromJson(_saveReply->readAll());
	QJsonObject jsonObject = doc.object();

	// Notify the target of the success or failure
	if (_saveReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		_url = jsonObject["url"].toString();
		_name = jsonObject["name"].toString();
		_isDirty = false;
		emit saveCompleted(true, PFErrorPtr());
	}
	else // FAILURE
	{
		int errorCode = jsonObject["code"].toInt();
		QString errorMessage = jsonObject["error"].toString();
		emit saveCompleted(false, PFErrorPtr(new PFError(errorCode, errorMessage)));
	}

	// Clean up
	_saveReply->deleteLater();
}

#pragma mark - Protected Get Data Slots

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
	// Update our ivar
	_isDownloading = false;

	// Notify the target of the success or failure
	if (_getDataReply->error() == QNetworkReply::NoError) // SUCCESS
	{
		// Move the data from the temp download data object to the final data object
		_data = _tempDownloadData;
		_tempDownloadData.clear();

		// Write the data out to the cache
		QString filepath = PFManager::instance()->cacheDirectory().filePath(_name);
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
		emit getDataCompleted(NULL, PFErrorPtr(new PFError(errorCode, errorMessage)));
	}

	// Clean up
	_getDataReply->deleteLater();
}

#pragma mark - Protected Methods

QString PFFile::convertExtensionToMimeType(const QString& extension)
{
	if (extension == "json")
		return "application/json";
	else if (extension == "pdf")
		return "application/pdf";
	else if (extension == "xml")
		return "application/xml";
	else if (extension == "zip")
		return "application/zip";
	else if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	else if (extension == "png")
		return "image/png";
	else if (extension == "txt")
		return "text/plain";
	else
		return "";
}

}	// End of parse namespace
