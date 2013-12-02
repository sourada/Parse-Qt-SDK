//
//  PFConversion.cpp
//  Parse
//
//  Created by Christian Noon on 11/27/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

// Parse headers
#include "PFConversion.h"
#include "PFDateTime.h"
#include "PFFile.h"
#include "PFObject.h"
#include "PFUser.h"

// Qt headers
#include <QJsonArray>
#include <QJsonDocument>

namespace parse {

namespace PFConversion {

#pragma mark - Recursive Conversion Methods

QJsonValue convertVariantToJson(const QVariant& variant)
{
	if ((QMetaType::Type) variant.type() == QMetaType::QVariantList)
	{
		QJsonArray jsonArray;
		foreach (const QVariant& dataObject, variant.toList())
		{
			QJsonValue jsonValue = convertVariantToJson(dataObject);
			jsonArray.append(jsonValue);
		}

		return QJsonValue(jsonArray);
	}
	else if ((QMetaType::Type) variant.type() == QMetaType::QVariantMap)
	{
		QJsonObject jsonObject;
		QVariantMap dataMap = variant.toMap();
		foreach (const QString& key, dataMap.keys())
		{
			QVariant dataObject = dataMap[key];
			jsonObject[key] = convertVariantToJson(dataObject);
		}

		return QJsonValue(jsonObject);
	}
	else if ((QMetaType::Type) variant.type() == QMetaType::QVariantHash)
	{
		QJsonObject jsonObject;
		QVariantHash dataHash;
		foreach (const QString& key, dataHash.keys())
		{
			QVariant dataObject = dataHash[key];
			jsonObject[key] = convertVariantToJson(dataObject);
		}

		return QJsonValue(jsonObject);
	}
	else if (variant.canConvert<PFSerializablePtr>())	// PFSerializablePtr
	{
		PFSerializablePtr serializable = variant.value<PFSerializablePtr>();
		QJsonObject jsonObject;
		serializable->toJson(jsonObject);
		return QJsonValue(jsonObject);
	}
	else
	{
		return QJsonValue::fromVariant(variant);
	}
}

QVariant convertJsonToVariant(const QJsonValue& jsonValue)
{
	if (jsonValue.type() == QJsonValue::Array)
	{
		QVariantList variantList;
		QJsonArray jsonArray = jsonValue.toArray();
		foreach (const QJsonValue& arrayJsonValue, jsonArray)
		{
			QVariant arrayVariant = convertJsonToVariant(arrayJsonValue);
			variantList.append(arrayVariant);
		}

		return variantList;
	}
	else if (jsonValue.type() == QJsonValue::Object)
	{
		QJsonObject jsonObject = jsonValue.toObject();
		if (jsonObject.contains("__type")) // Some PF* class
		{
			QString objectType = jsonObject["__type"].toString();
			if (objectType == "Date")
			{
				return PFDateTime::fromJson(jsonObject);
			}
			else if (objectType == "File")
			{
				return PFFile::fromJson(jsonObject);
			}
			else if (objectType == "Pointer")
			{
				QString className = jsonObject["className"].toString();
				if (className == "_User")
				{
					return PFUser::fromJson(jsonObject);
				}
				else
				{
					return PFObject::fromJson(jsonObject);
				}
			}
			else
			{
				return QVariant();
			}
		}
		else // Contains a map of stuff so recursively convert it some more
		{
			QVariantMap variantMap;
			foreach (const QString& jsonKey, jsonObject.keys())
			{
				QJsonValue dictionaryJsonValue = jsonObject[jsonKey];
				variantMap[jsonKey] = convertJsonToVariant(dictionaryJsonValue);
			}

			return variantMap;
		}
	}
	else
	{
		return jsonValue.toVariant();
	}
}

#pragma mark - Conversion Methods

bool areEqual(const QVariant& variant1, const QVariant& variant2)
{
	// First try the default comparison
	if (variant1 == variant2)
		return true;

	// Compare the JSON values if we have serializables
	PFSerializablePtr serializable1 = variant1.value<PFSerializablePtr>();
	PFSerializablePtr serializable2 = variant2.value<PFSerializablePtr>();
	if (!serializable1.isNull() && !serializable2.isNull())
	{
		// Make sure that they're the same class before converting to JSON
		if (serializable1->pfClassName() == serializable2->pfClassName())
		{
			QJsonObject jsonVariant1, jsonVariant2;
			serializable1->toJson(jsonVariant1);
			serializable2->toJson(jsonVariant2);
			QByteArray json1 = QJsonDocument(jsonVariant1).toJson();
			QByteArray json2 = QJsonDocument(jsonVariant2).toJson();
			return (json1 == json2);
		}
	}

	return false;
}

}	// End of PFConversion namespace

}	// End of parse namespace
