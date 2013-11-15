//
//  PFDateTime.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFDATETIME_H
#define PARSE_PFDATETIME_H

// Parse headers
#include "PFSerializable.h"

// Qt headers
#include <QDateTime>

namespace parse {

class PFDateTime : public PFSerializable
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	// Creation Methods
	static PFDateTimePtr dateTimeFromParseString(const QString& parseString);
	static PFDateTimePtr dateTimeFromDateTime(const QDateTime& dateTime);
	static PFDateTimePtr dateTimeFromVariant(const QVariant& variant);

	// DateTime Access
	QString toParseString() const;
	const QDateTime& dateTime() const;

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

	// PFSerializable Methods
	static QVariant fromJson(const QJsonObject& jsonObject);
	virtual bool toJson(QJsonObject& jsonObject);
	virtual const QString className() const;

protected:

	// Constructor / Destructor
	PFDateTime(const QDateTime& dateTime);
	virtual ~PFDateTime();

	// Instance members
	QDateTime _dateTime;
};

}	// End of parse namespace

QDebug operator<<(QDebug dbg, const parse::PFDateTimePtr& dateTime);

Q_DECLARE_METATYPE(parse::PFDateTimePtr)

#endif	// End of PARSE_PFDATETIME_H
