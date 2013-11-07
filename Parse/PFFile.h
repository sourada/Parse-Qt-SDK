//
//  PFFile.h
//  Parse
//
//  Created by Christian Noon on 11/6/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFFILE_H
#define PARSE_PFFILE_H

// Parse headers
#include <Parse/PFError.h>

// Qt headers
#include <QFile>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QString>

namespace parse {

// Typedefs
class PFFile;
typedef QSharedPointer<PFFile> PFFilePtr;

/**
 * USE CASES
 *
 * Saving Data:
 *    1. I want to upload a file in which I will call save
 *    2. Probably will need to be able to verify is has been saved into the cloud (isDirty() method)
 *    3. I will then want to attach that file to objects for associate then saving (toJSON() method)
 *
 * Getting Data:
 *    1. I will want to find out whether the data is available (is it cached)
 *    2. If not, then I will want to fetch it
 *    3. Once fetched, I will want to be able to access the data directly
 *
 * Cancelling Active Process:
 *    1. User may want to cancel an upload
 *    2. User may want to cacnel a download
 */

class PFFile : public QObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	/**
	 * Constructor.
	 * Use to save data to server.
	 */
	PFFile(const QString& filepath);

	/**
	 * Constructor.
	 * Use to get data from cache or server.
	 */
	PFFile(const QString& name, const QString& url);

	/** Destructor. */
	~PFFile();

	/**
	 * Returns whether the data has been saved to the server.
	 * @return True if the data has been saved to the server, false otherwise.
	 */
	bool isDirty();

	/**
	 * Saves the data asynchronously to the server.
	 * @param saveProgressTarget The target to be notified when the save progress changes.
	 * @param saveProgressAction The slot to be notified when the save progress changes - SLOT(saveProgressUpdated(double)).
	 * @param saveCompleteTarget The target to be notified when the save completes.
	 * @param saveCompleteAction The slot to be notified when the save completes - SLOT(saveCompleted(bool, PFErrorPtr)).
	 * @return True if the async save process was started, false otherwise.
	 */
	bool saveInBackground(QObject *saveProgressTarget, const char *saveProgressAction, QObject *saveCompleteTarget, const char *saveCompleteAction);

	/**
	 * Returns whether the data is available in memory or in the cache.
	 * NOTE: if it is not available, then it needs to be downloaded from the server.
	 * @return True if the data is available in memory or in the cache, false otherwise.
	 */
	bool isDataAvailable();

	/**
	 * Gets the data from memory or from the cache.
	 * NOTE: if the data is not available, then this method returns NULL. If this happens,
	 * then use the getDataInBackground() method to download the data from the server.
	 * @return The data from memory or from the cache.
	 */
	QByteArray* getData();

	/**
	 * Gets the data from the server, saves it to the cache then delivers the data to the target slot.
	 * @param getDataProgressTarget The target to be notified when the get data progress changes.
	 * @param getDataProgressAction The slot to be notified when the get data progress changes - SLOT(getDataProgressUpdated(double)).
	 * @param getDataCompleteTarget The target to be notified when the get data completes.
	 * @param getDataCompleteAction The slot to be notified when the get data completes - SLOT(getDataCompleted(bool, PFErrorPtr)).
	 * @return True if the async get data process was started, false otherwise.
	 */
	bool getDataInBackground(QObject *getDataProgressTarget, const char *getDataProgressAction, QObject *getDataCompleteTarget, const char *getDataCompleteAction);

	/** Cancels the current request (whether uploading or downloading the file data. */
	void cancel();

	/** Returns the filepath of the original file data. */
	const QString& filepath();

	/** Returns the name of the file stored on the server. */
	const QString& name();

	/** Returns the url of the file stored on the server. */
	const QString& url();

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

protected slots:

	// Save Slots
	void handleSaveProgressUpdated(qint64 bytesSent, qint64 bytesTotal);
	void handleSaveCompleted();

	// Get Data Slots
	void handleGetDataReadyRead();
	void handleGetDataProgressUpdated(qint64 bytesSent, qint64 bytesTotal);
	void handleGetDataCompleted();

signals:

	// Save Signals
	void saveProgressUpdated(double percentDone);
	void saveCompleted(bool succeeded, PFErrorPtr error);

	// Get Data Signals
	void getDataProgressUpdated(double percentDone);
	void getDataCompleted(QByteArray* data, PFErrorPtr error);

protected:

	QString convertExtensionToMimeType(const QString& extension);

	/** Instance members. */
	QString							_filepath;
	QString							_mimeType;
	QString							_name;
	QString							_url;
	QSharedPointer<QByteArray>		_data;
	QSharedPointer<QByteArray>		_tempDownloadData;
	bool							_isDirty;
	bool							_isUploading;
	bool							_isDownloading;
	QNetworkReply*					_saveReply;
	QNetworkReply*					_getDataReply;
};

}	// End of parse namespace

#endif	// End of PARSE_PFFILE_H
