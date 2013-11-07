//
//  PFDateTime.cpp
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include "PFDateTime.h"

namespace parse {

// Static Globals
static QString gParseDateFormat = "yyyy-MM-ddTHH:mm:ss.zzzZ";

PFDateTime::PFDateTime(const QDateTime& localTime)
{
	_dateTime = localTime.toUTC();
}

PFDateTime::PFDateTime(const QString& parseString)
{
	_dateTime = QDateTime::fromString(parseString, gParseDateFormat);
	_dateTime.setTimeSpec(Qt::UTC);
}

PFDateTime::~PFDateTime()
{
	// No-op
}

QString PFDateTime::toParseString()
{
	return _dateTime.toString(gParseDateFormat);
}

QDateTime PFDateTime::toLocalTime()
{
	return _dateTime.toLocalTime();
}

QDateTime PFDateTime::toUTC()
{
	return _dateTime;
}

}	// End of parse namespace
