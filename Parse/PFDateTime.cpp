//
//  PFDateTime.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// C++ headers
#include <assert.h>

// Parse headers
#include "PFDateTime.h"

// Qt headers
#include <QJsonObject>

namespace parse {

// Static Globals
static QString gParseDateFormat = "yyyy-MM-ddTHH:mm:ss.zzzZ";

#pragma mark - Memory Management Methods

PFDateTime::PFDateTime() : QDateTime()
{
	// No-op
}

PFDateTime::PFDateTime(const QDateTime& dateTime) : QDateTime(dateTime)
{
	// No-op
}

PFDateTime::~PFDateTime()
{
	// No-op
}

QVariant PFDateTime::variantWithDateTime(const PFDateTime& dateTime)
{
	QVariant variant;
	variant.setValue(dateTime);
	return variant;
}

#pragma mark - Conversion Helpers

PFDateTime PFDateTime::fromParseString(const QString& parseString)
{
	PFDateTime dateTime = QDateTime::fromString(parseString, gParseDateFormat);
	dateTime.setTimeSpec(Qt::UTC);

	return dateTime;
}

QString PFDateTime::toParseString() const
{
	// Create a copy to manipulate
	PFDateTime localThis = *this;

	// Make sure time spec is utf
	if (localThis.timeSpec() != Qt::UTC)
		localThis.setTimeSpec(Qt::UTC);

	return localThis.toString(gParseDateFormat);
}

#pragma mark - PFSerializable Methods

void PFDateTime::fromJson(const QJsonObject& jsonObject)
{
	qDebug() << "PFDateTime::fromJson";
	QString utcDateString = jsonObject["iso"].toString();
	*this = PFDateTime::fromParseString(utcDateString);
}

void PFDateTime::toJson(QJsonObject& jsonObject)
{
	qDebug() << "PFDateTime::toJson";
	if (!isValid())
		qFatal("PFDateTime::toJson could NOT convert PFDateTime to JSON because it is NOT valid");

	jsonObject["__type"] = QString("Date");
	jsonObject["iso"] = toParseString();
}

}	// End of parse namespace

#pragma mark - Custom Debug Output

QDebug operator<<(QDebug dbg, const parse::PFDateTime& dateTime)
{
	QString timeSpecText;
	if (dateTime.timeSpec() == Qt::LocalTime)
		timeSpecText = "Qt::LocalTime";
	else if (dateTime.timeSpec() == Qt::UTC)
		timeSpecText = "Qt::UTC";
	else if (dateTime.timeSpec() == Qt::OffsetFromUTC)
		timeSpecText = "Qt::OffsetFromUTC";
	else
		timeSpecText = "Qt::TimeZone";

	QString dateText = dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz t") + " " + timeSpecText;
	dbg.nospace() << "PFDateTime(" << dateText << ")";

	return dbg;
}
