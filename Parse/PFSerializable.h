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

/** Abstract class forcing the serializable api on all subclasses. */
class PFSerializable : public QObject
{
public:

	// Serialization Methods
	virtual bool toJson(QJsonObject& jsonObject) = 0;
	virtual const QString className() const = 0;

	// Variant Conversion Helpers
	static QVariant toVariant(PFSerializablePtr serializable);
	static PFSerializablePtr fromVariant(const QVariant& variant);

protected:

	PFSerializable();
	virtual ~PFSerializable();
};

}	// End of parse namespace

Q_DECLARE_METATYPE(parse::PFSerializablePtr)

#endif	// End of PARSE_PFSERIALIZABLE_H
