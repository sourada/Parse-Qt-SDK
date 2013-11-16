//
//  PFSerializable.cpp
//  Parse
//
//  Created by Christian Noon on 11/11/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

// Parse headers
#include "PFSerializable.h"

namespace parse {

#pragma mark - Memory Management Methods

PFSerializable::PFSerializable() : QObject()
{
	// No-op
}

PFSerializable::~PFSerializable()
{
	// No-op
}

#pragma mark - Serialization Methods

QVariant PFSerializable::toVariant(PFSerializablePtr serializable)
{
	QVariant variant;
	variant.setValue(serializable);
	return variant;
}

}	// End of parse namespace
