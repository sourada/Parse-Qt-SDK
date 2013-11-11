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
#include <Parse/PFSerializable.h>

// Qt headers
#include <QDateTime>
#include <QMetaType>

namespace parse {

class PFDateTime : public QDateTime, public PFSerializable
{
public:

	// Construction / Destruction
	PFDateTime();
	PFDateTime(const QDateTime& dateTime);
	virtual ~PFDateTime();

	// Variant helper
	static QVariant variantWithDateTime(const PFDateTime& dateTime);

	// Conversion helpers to switch between Parse and Qt timestamps
	static PFDateTime fromParseString(const QString& parseString);
	QString toParseString() const;

	// PFSerializable Methods
	void fromJson(const QJsonObject& jsonObject);
	void toJson(QJsonObject& jsonObject);
};

}	// End of parse namespace

QDebug operator<<(QDebug dbg, const parse::PFDateTime& dateTime);

Q_DECLARE_METATYPE(parse::PFDateTime)

#endif	// End of PARSE_PFDATETIME_H
