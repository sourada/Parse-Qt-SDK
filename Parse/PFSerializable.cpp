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

PFSerializable::PFSerializable() : QObject()
{
	// No-op
}

PFSerializable::~PFSerializable()
{
	// No-op
}

QVariant PFSerializable::toVariant(PFSerializablePtr serializable)
{
	QVariant variant;
	variant.setValue(serializable);
	return variant;
}

PFSerializablePtr PFSerializable::fromVariant(const QVariant& variant)
{
	return variant.value<PFSerializablePtr>();
}

}	// End of parse namespace
