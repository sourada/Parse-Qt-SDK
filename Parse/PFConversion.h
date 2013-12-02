//
//  PFConversion.h
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#ifndef PARSE_PFCONVERSION_H
#define PARSE_PFCONVERSION_H

// Qt headers
#include <QJsonValue>
#include <QVariant>

namespace parse {

namespace PFConversion {

// Recursive Conversion Methods
QJsonValue convertVariantToJson(const QVariant& variant);
QVariant convertJsonToVariant(const QJsonValue& jsonValue);

// Comparison Methods
bool areEqual(const QVariant& variant1, const QVariant& variant2);

}	// End of PFConversion namespace

}	// End of parse namespace

#endif	// End of PARSE_PFCONVERSION_H
