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

namespace parse {

class PFDateTime : public QDateTime
{
public:

	PFDateTime();
	PFDateTime(const QDateTime& dateTime);
	~PFDateTime();

	static PFDateTime fromParseString(const QString& parseString);
	QString toParseString();
};

}	// End of parse namespace

#endif	// End of PARSE_PFDATETIME_H
