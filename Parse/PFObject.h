//
//  PFObject.h
//  Parse
//
//  Created by Christian Noon on 11/7/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFOBJECT_H
#define PARSE_PFOBJECT_H

// Parse headers
#include <Parse/PFDateTime.h>
#include <Parse/PFError.h>

// Qt headers
#include <QHash>
#include <QNetworkReply>

namespace parse {

// Typedefs
class PFObject;
typedef QSharedPointer<PFObject> PFObjectPtr;

class PFObject : public QObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Destructor
	~PFObject();

	// Creation methods
	static PFObjectPtr objectWithClassName(const QString& className);
	static PFObjectPtr objectWithClassName(const QString& className, const QString& objectId);

	// Primitives
	void setObjectForKey(const QVariant& object, const QString& key);
	const QVariant& objectForKey(const QString& key);
	QList<QString> allKeys();

	// Accessors
	inline const QString& parseClassName() { return _parseClassName; }
	inline const QString& objectId() { return _objectId; }
	inline const PFDateTime& createdAt() { return _createdAt; }
	inline const PFDateTime& updatedAt() { return _updatedAt; }

	// Save
	bool save();
	bool save(PFErrorPtr& error);
	bool saveInBackground(QObject *saveCompleteTarget, const char *saveCompleteAction);

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

protected slots:

	// Save Slots
	void handleSaveCompleted();

signals:

	// Save Signals
	void saveCompleted(PFObject* object, bool succeeded, PFErrorPtr error);

protected:

	// Constructor
	PFObject(const QString& className);
	PFObject(const QString& className, const QString& objectId);

	// Default initialization
	void initialize();

	// Methods for constructing a save
	bool needsUpdated();
	QNetworkRequest buildSaveNetworkRequest();
	QByteArray buildSaveData();
	PFErrorPtr parseSaveNetworkReply(QNetworkReply* networkReply, bool updated);

	/** Instance members. */
	QString				_parseClassName;
	QString				_objectId;
	PFDateTime			_createdAt;
	PFDateTime			_updatedAt;
	QVariantMap			_primitiveObjects;
	QVariantMap			_updatedPrimitiveObjects;
	bool				_isSaving;
	QNetworkReply*		_saveReply;
};

}	// End of parse namespace

#endif	// End of PARSE_PFOBJECT_H
