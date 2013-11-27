//
//  PFQuery.h
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFQUERY_H
#define PARSE_PFQUERY_H

// Parse headers
#include "PFTypedefs.h"

namespace parse {

class PFQuery : public QObject
{
	Q_OBJECT

public:

	//=================================================================================
	//                                  USER API
	//=================================================================================

	//=================================================================================
	//                                BACKEND API
	//=================================================================================

protected:

	// Constructor / Destructor
	PFQuery();
	~PFQuery();

	// Instance members
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFQueryPtr)

#endif	// End of PARSE_PFQUERY_H
