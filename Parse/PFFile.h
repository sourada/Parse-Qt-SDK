//
//  PFFile.h
//  Parse
//
//  Created by Christian Noon on 11/6/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFFILE_H
#define PARSE_PFFILE_H

// Parse headers
#include "PFSerializable.h"

// Qt headers
#include <QFile>
#include <QNetworkReply>
#include <QString>

namespace parse {

class PFFile : public PFSerializable
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creation Methods for Upload
	static PFFilePtr fileWithData(QByteArrayPtr data);
	static PFFilePtr fileWithNameAndData(const QString& name, QByteArrayPtr data);
	static PFFilePtr fileWithNameAndContentsAtPath(const QString& name, const QString& filepath);
	static PFFilePtr fileFromVariant(const QVariant& variant);

	// Creation Methods for Download
	static PFFilePtr fileWithNameAndUrl(const QString& name, const QString& url);

	// Getter Methods
	const QString& filepath();
	const QString& name();
	const QString& url();

	// Returns whether the data has been saved to the server
	bool isDirty();

	////////////////////////////////
	//       Save Methods
	////////////////////////////////

	// Saves the data synchronously to the server
	bool save();
	bool save(PFErrorPtr& error);

	// Saves the data asynchronously to the server (use the second method for receiving progress updates).
	//   @param saveProgressTarget The target to be notified when the save progress changes.
	//   @param saveProgressAction The slot to be notified when the save progress changes - SLOT(saveProgressUpdated(double)).
	//   @param saveCompleteTarget The target to be notified when the save completes.
	//   @param saveCompleteAction The slot to be notified when the save completes - SLOT(saveCompleted(bool, PFErrorPtr)).
	//   @return True if the async save process was started, false otherwise.
	bool saveInBackground(QObject *target = 0, const char *action = 0);
	bool saveInBackground(QObject *saveProgressTarget, const char *saveProgressAction,
						  QObject *saveCompleteTarget, const char *saveCompleteAction);

	////////////////////////////////
	//     Check File Methods
	////////////////////////////////

	// Checks the url synchronously to see if the data exists on the server
	bool checkUrlForFile();

	// Checks the url asynchronously to see if the data exists on the server
	//   @param target The target to be notified when the check completes.
	//   @param action The slot to be notified when the check completes - SLOT(checkUrlForFileCompleted(bool)).
	//   @return True if the async check process was started, false otherwise.
	bool checkUrlForFileInBackground(QObject *target, const char *action);

	////////////////////////////////
	//     Get Data Methods
	////////////////////////////////

	// Returns whether the data is available in memory or in the cache.
	// NOTE: if it is not available, then it needs to be downloaded from the server.
	bool isDataAvailable();

	// Gets the data from memory or from the cache.
	// NOTE: if the data is not available, then this method returns NULL. If this happens,
	// then use the getDataInBackground() method to download the data from the server.
	QByteArray* getData();

	// Gets the data from the server, saves it to the cache then delivers the data to the target slot.
	//   @param getDataProgressTarget The target to be notified when the get data progress changes.
	//   @param getDataProgressAction The slot to be notified when the get data progress changes - SLOT(getDataProgressUpdated(double)).
	//   @param getDataCompleteTarget The target to be notified when the get data completes.
	//   @param getDataCompleteAction The slot to be notified when the get data completes - SLOT(getDataCompleted(bool, PFErrorPtr)).
	//   @return True if the async get data process was started, false otherwise.
	bool getDataInBackground(QObject *target = 0, const char *action = 0);
	bool getDataInBackground(QObject *getDataProgressTarget, const char *getDataProgressAction,
							 QObject *getDataCompleteTarget, const char *getDataCompleteAction);

	////////////////////////////////
	//       Delete Methods
	////////////////////////////////

	// Delete the data synchronously from the server
	bool deleteFile();
	bool deleteFile(PFErrorPtr& error);

	// Deletes the file asynchronously from the server.
	//   @param deleteCompleteTarget The target to be notified when the delete completes.
	//   @param deleteCompleteAction The slot to be notified when the delete completes - SLOT(deleteCompleted(bool, PFErrorPtr)).
	//   @return True if the async delete process was started, false otherwise.
	bool deleteFileInBackground(QObject *target = 0, const char *action = 0);

	////////////////////////////////
	//       Cancellation
	////////////////////////////////

	// Cancels the current request (whether uploading or downloading the file data
	void cancel();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	static QVariant fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString pfClassName() const;

protected slots:

	// Save Slots
	void handleSaveProgressUpdated(qint64 bytesSent, qint64 bytesTotal);
	void handleSaveCompleted();

	// Check Url for File Slots
	void handleCheckUrlForFileCompleted();

	// Get Data Slots
	void handleGetDataReadyRead();
	void handleGetDataProgressUpdated(qint64 bytesSent, qint64 bytesTotal);
	void handleGetDataCompleted();

	// Delete Slots
	void handleDeleteCompleted();

signals:

	// Save Signals
	void saveProgressUpdated(double percentDone);
	void saveCompleted(bool succeeded, PFErrorPtr error);

	// Check Url Signals
	void checkUrlForFileCompleted(bool succeeded);

	// Get Data Signals
	void getDataProgressUpdated(double percentDone);
	void getDataCompleted(QByteArray* data, PFErrorPtr error);

	// Delete Signals
	void deleteCompleted(bool succeeded, PFErrorPtr error);

protected:

	// Constructor / Destructor
	PFFile();
	~PFFile();

	// Network Request Builder Methods
	QNetworkRequest createSaveNetworkRequest();
	QNetworkRequest createCheckUrlForFileNetworkRequest();
	QNetworkRequest createDeleteNetworkRequest();

	// Network Reply Deserialization Methods
	bool deserializeSaveNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);
	bool deserializeCheckUrlForFileNetworkReply(QNetworkReply* networkReply);
	bool deserializeDeleteNetworkReply(QNetworkReply* networkReply, PFErrorPtr& error);

	// Instance members
	QString				_filepath;
	QString				_mimeType;
	QString				_name;
	QString				_url;
	QByteArrayPtr		_data;
	QByteArrayPtr		_tempDownloadData;
	bool				_isDirty;
	bool				_isUploading;
	bool				_isDownloading;
	bool				_isDeleting;
	QNetworkReply*		_saveReply;
	QNetworkReply*		_checkUrlReply;
	QNetworkReply*		_getDataReply;
	QNetworkReply*		_deleteReply;
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFFilePtr)

#endif	// End of PARSE_PFFILE_H
