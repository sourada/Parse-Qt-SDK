//
//  PFDateTime.h
//  Parse
//
//  Created by Christian Noon on 11/5/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFDATETIME_H
#define PARSE_PFDATETIME_H

// Qt headers
#include <QDateTime>
#include <QSharedPointer>

namespace parse {

// Typedefs
class PFDateTime;
typedef QSharedPointer<PFDateTime> PFDateTimePtr;

// A PFDateTime object will always store the backing date as UTC
class PFDateTime
{
public:

	PFDateTime(const QDateTime& localTime);
	PFDateTime(const QString& parseString);
	~PFDateTime();

	QString toParseString();
	QDateTime toLocalTime();
	QDateTime toUTC();

protected:

	/** Instance member variables. */
	QDateTime	_dateTime;
};

}	// End of parse namespace

#endif	// End of PARSE_PFDATETIME_H
