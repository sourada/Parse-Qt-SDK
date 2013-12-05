//
//  PFDateTime.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFDateTime.h"

namespace parse {

// Static Globals
static QString gParseDateFormat = "yyyy-MM-ddTHH:mm:ss.zzzZ";

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFDateTime::PFDateTime(const QDateTime& dateTime) :
	_dateTime(dateTime)
{
	qDebug().nospace() << "Created PFDateTime(" << QString().sprintf("%8p", this) << ")";
}

PFDateTime::~PFDateTime()
{
	qDebug().nospace() << "Destroyed PFDateTime(" << QString().sprintf("%8p", this) << ")";
}

#ifdef __APPLE__
#pragma mark - Static Creation Methods
#endif

PFDateTimePtr PFDateTime::dateTimeFromParseString(const QString& parseString)
{
	QDateTime dateTime = QDateTime::fromString(parseString, gParseDateFormat);
	if (!dateTime.isValid())
	{
		qWarning() << "PFDateTime::dateTimeFromParseString failed due to an invalid parse string" << parseString;
		return PFDateTimePtr();
	}
	else
	{
		dateTime.setTimeSpec(Qt::UTC);
		PFDateTimePtr pfDateTime = PFDateTimePtr(new PFDateTime(dateTime), &QObject::deleteLater);
		return pfDateTime;
	}
}

PFDateTimePtr PFDateTime::dateTimeFromDateTime(const QDateTime& dateTime)
{
	if (!dateTime.isValid())
	{
		qWarning("PFDateTime::dateTimeFromDateTime failed due to an invalid date time");
		return PFDateTimePtr();
	}
	else
	{
		QDateTime nonConstDateTime = dateTime;
		nonConstDateTime.setTimeSpec(Qt::UTC);
		PFDateTimePtr pfDateTime = PFDateTimePtr(new PFDateTime(nonConstDateTime), &QObject::deleteLater);
		return pfDateTime;
	}
}

PFDateTimePtr PFDateTime::dateTimeFromVariant(const QVariant& variant)
{
	PFSerializablePtr serializable = variant.value<PFSerializablePtr>();
	if (!serializable.isNull())
		return serializable.objectCast<PFDateTime>();

	return PFDateTimePtr();
}

#ifdef __APPLE__
#pragma mark - DateTime Access Methods
#endif

QString PFDateTime::toParseString() const
{
	return _dateTime.toString(gParseDateFormat);
}

const QDateTime& PFDateTime::dateTime() const
{
	return _dateTime;
}

#ifdef __APPLE__
#pragma mark - PFSerializable Methods
#endif

QVariant PFDateTime::fromJson(const QJsonObject& jsonObject)
{
	QString parseDateString = jsonObject["iso"].toString();
	PFDateTimePtr dateTime = PFDateTime::dateTimeFromParseString(parseDateString);
	return toVariant(dateTime);
}

bool PFDateTime::toJson(QJsonObject& jsonObject)
{
	jsonObject["__type"] = QString("Date");
	jsonObject["iso"] = toParseString();
	return true;
}

const QString PFDateTime::pfClassName() const
{
	return "PFDateTime";
}

}	// End of parse namespace

#ifdef __APPLE__
#pragma mark - Custom Debug Output
#endif

QDebug operator<<(QDebug dbg, const parse::PFDateTimePtr& dateTime)
{
	QString timeSpecText;
	QDateTime qDateTime = dateTime->dateTime();
	if (qDateTime.timeSpec() == Qt::LocalTime)
		timeSpecText = "Qt::LocalTime";
	else if (qDateTime.timeSpec() == Qt::UTC)
		timeSpecText = "Qt::UTC";
	else if (qDateTime.timeSpec() == Qt::OffsetFromUTC)
		timeSpecText = "Qt::OffsetFromUTC";
	else
		timeSpecText = "Qt::TimeZone";

	QString dateText = qDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz t") + " " + timeSpecText;
	dbg.nospace() << "PFDateTime(" << dateText << ")";

	return dbg;
}
