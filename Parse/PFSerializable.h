//
//  PFSerializable.h
//  Parse
//
//  Created by Christian Noon on 11/8/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFSERIALIZABLE_H
#define PARSE_PFSERIALIZABLE_H

// Qt headers
#include <QJsonObject>
#include <QVariant>

namespace parse {

/** Abstract class forcing the serializable api on all subclasses. */
class PFSerializable
{
public:

	virtual void fromJson(const QJsonObject& jsonObject) = 0;
	virtual void toJson(QJsonObject& jsonObject) = 0;
};

}	// End of parse namespace

#endif	// End of PARSE_PFSERIALIZABLE_H
