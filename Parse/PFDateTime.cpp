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

PFDateTime PFDateTime::fromParseString(const QString& parseString)
{
	PFDateTime dateTime = QDateTime::fromString(parseString, gParseDateFormat);
	dateTime.setTimeSpec(Qt::UTC);

	return dateTime;
}

QString PFDateTime::toParseString()
{
	// Make sure time spec is utf
	if (timeSpec() != Qt::UTC)
		setTimeSpec(Qt::UTC);

	return toString(gParseDateFormat);
}

}	// End of parse namespace
