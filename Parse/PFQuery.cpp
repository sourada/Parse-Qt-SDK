//
//  PFQuery.cpp
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFQuery.h"

// Qt headers
#include <QDebug>

namespace parse {

PFQuery::PFQuery()
{
	qDebug().nospace() << "Created PFQuery(" << QString().sprintf("%8p", this) << ")";
}

PFQuery::~PFQuery()
{
	qDebug().nospace() << "Destroyed PFQuery(" << QString().sprintf("%8p", this) << ")";
}

}	// End of parse namespace
