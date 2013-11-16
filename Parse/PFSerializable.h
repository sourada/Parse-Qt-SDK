//
//  PFSerializable.h
//  Parse
//
//  Created by Christian Noon on 11/8/13.
//  Copyright (c) 2013 BodyViz. All rights reserved.
//

#ifndef PARSE_PFSERIALIZABLE_H
#define PARSE_PFSERIALIZABLE_H

// Parse headers
#include "PFTypedefs.h"

// Qt headers
#include <QDebug>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

namespace parse {

class PFSerializable : public QObject
{
public:

	// Serialization Methods (subclass also needs to implement a static fromJson() method)
	virtual bool toJson(QJsonObject& jsonObject) = 0;
	virtual const QString pfClassName() const = 0;

	// Converting to a variant
	static QVariant toVariant(PFSerializablePtr serializable);

protected:

	// Constructor / Destructor
	PFSerializable();
	virtual ~PFSerializable();
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFSerializablePtr)

#endif	// End of PARSE_PFSERIALIZABLE_H
