//
//  PFSerializable.cpp
//  Parse
//
//  Created by Christian Noon on 11/11/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFSerializable.h"

namespace parse {

#ifdef __APPLE__
#pragma mark - Memory Management Methods
#endif

PFSerializable::PFSerializable() : QObject()
{
	// No-op
}

PFSerializable::~PFSerializable()
{
	// No-op
}

#ifdef __APPLE__
#pragma mark - Serialization Methods
#endif

QVariant PFSerializable::toVariant(PFSerializablePtr serializable)
{
	QVariant variant;
	variant.setValue(serializable);
	return variant;
}

}	// End of parse namespace
