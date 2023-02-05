/*! \file MixedValue.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class MixedValue
 *  \section license License
 *
 *  Copyright (C) 2009 Jason Hogan <hogan@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *
 *  The STI is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The STI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the STI.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/variant.hpp>

#include <sstream>
#include <iostream>
#include <algorithm>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueVector;
using STI::Utils::MixedValueType;


MixedValue::MixedValue()
{
	type = MixedValueType::Empty;
}
MixedValue::MixedValue(const MixedValue& copy)
{
	setValueMixed(copy);
}

MixedValue::MixedValue(const MixedValueType& valueType)
{
	if (valueType == MixedValueType::Empty) {
		type = MixedValueType::Empty;
	}
	else {
		type = valueType;
		// setValue(valueType);	//error
	}
}

MixedValue::~MixedValue()
{
}


bool MixedValue::operator==(const MixedValue& other) const
{
	if(type != other.getType())
		return false;

	// return other.value_v == value_v;

	bool result = false;

	switch(type)
	{
	case MixedValueType::Empty:
		result = true;	//both are empty
	case MixedValueType::Boolean:
		result = ( getBoolean() == other.getBoolean() );
		break;
	case MixedValueType::Int:
		result = ( getInt() == other.getInt() );
		break;
	case MixedValueType::Double:
		result = ( getDouble() == other.getDouble() );
		break;
	case MixedValueType::String:
		result = ( getString().compare(other.getString()) == 0 );
		break;
	case MixedValueType::Vector:
		{
			const MixedValueVector& values = getVector();
			const MixedValueVector& otherValues = other.getVector();

			if(values.size() != otherValues.size()) {
				result = false;
			}
			else {
				result = true;

				for(unsigned i = 0; (i < otherValues.size() && i < values.size()); i++)
				{
					result &= ( values.at(i) == otherValues.at(i) );
				}
			}
		}
		break;
	case MixedValueType::VectorInt:
		{
			const std::vector<int>* values;
			const std::vector<int>* otherValues;

			if (getFlatVector(values) && other.getFlatVector(otherValues)) {
				result = (*values) == (*otherValues);
			}
		}
		break;

	default:
		//this should never happen
		result = false;
		break;
	}

	return result;
}

bool MixedValue::operator!=(const MixedValue& other) const
{
	return !( (*this) == other );
}

void MixedValue::swap(MixedValue& other)
{
	std::swap(type, other.type);
	value_v.swap(other.value_v);
}

void MixedValue::setValue(bool value)
{
	clear();

	value_v = value;
	type = MixedValueType::Boolean;
}

void MixedValue::setValue(int value)
{
	clear();

	value_v = value;
	type = MixedValueType::Int;
}


void MixedValue::setValue(double value)
{
	clear();

	value_v = value;
	type = MixedValueType::Double;
}

void MixedValue::setValue(const std::string& value)
{
	clear();

	value_v = value;
	type = MixedValueType::String;
}

void MixedValue::setValue(const std::shared_ptr<STI::Utils::FileHolder>& value)
{
	clear();

	value_v = value;
	type = MixedValueType::File;
}

void MixedValue::setValue(const MixedValue& value)
{
	setValueMixed(value);
}

void MixedValue::setValueMixed(const MixedValue& value)
{
	//{ Empty, Boolean, Int, Double, String, Vector, VectorInt, File, Image, Any}

	switch( value.getType() )
	{
	case MixedValueType::Empty:
		setValue();
		break;
	case MixedValueType::Boolean:
		setValue( value.getBoolean() );
		break;
	case MixedValueType::Int:
		setValue( value.getInt() );
		break;
	case MixedValueType::Double:
		setValue( value.getDouble() );
		break;
	case MixedValueType::String:
		setValue( value.getString() );
		break;
	case MixedValueType::Vector:
		setValue( value.getVector() );
		break;
	case MixedValueType::VectorInt:
		{
			const std::vector<int>* values;
			if (value.getFlatVector(values)) {
				setValue(*values);
			}
		}
		// setValue( value.getVector() );
		break;
	case MixedValueType::File:
		setValue( value.getFile() );
		break;
	default:
		//this should never happen
		break;
	}

}

void MixedValue::setValue()
{
	clear();
	// type = MixedValueType::Empty;
}


void MixedValue::addValue(const MixedValue& value)
{
	convertToVector<MixedValue>();

	try {
		auto& values = std::get<MixedValueVector>(value_v);
		values.push_back(value);
	}
	catch (const std::bad_variant_access& ex) {
	}
}


void MixedValue::addValue(const int& value)
{
	convertToVector<int>();

	bool success = false;

	if (isType(MixedValueType::VectorInt)) {
		try {
			auto& values = std::get<std::vector<int>>(value_v);
			values.push_back(value);
			success = true;
		}
		catch (const std::bad_variant_access& ex) {
		}
	}

	if (!success) {
		MixedValue mixedValue;
		mixedValue.setValue(value);
		addValue(mixedValue);
	}
}

void MixedValue::wrapThenAppend(const MixedValue& source, MixedValueVector& target)
{
	if (source.isType(MixedValueType::Vector)) {
		auto& sourceVec = source.getVector();
		target.insert(target.end(), std::make_move_iterator(sourceVec.begin()), std::make_move_iterator(sourceVec.end()));
	}
	else if (source.isType(MixedValueType::VectorInt)) {
		const std::vector<int>* sourceVec;
		if (source.getFlatVector(sourceVec)) {
			for (auto& v : *sourceVec) {
				target.push_back(MixedValue());
				target.back().setValue(v);
			}
		}
	}
}

void MixedValue::clear()
{
	type = MixedValueType::Empty;
	value_v = std::monostate{};
}

MixedValueType MixedValue::getType() const
{
	return type;
}

bool MixedValue::isType(const MixedValueType& mixedValueType) const
{
	return type == mixedValueType;
}

bool MixedValue::getBoolean() const
{
	if (type == MixedValueType::Boolean) {
		try {
			auto& result = std::get<bool>(value_v);
			return result;
		}
		catch (const std::bad_variant_access& ex) {
		}
	}
	return (getNumber() != 0);
}

int MixedValue::getInt() const
{
	if (type == MixedValueType::Int) {
		try {
			auto& result = std::get<int>(value_v);
			return result;
		}
		catch (const std::bad_variant_access& ex) {
		}
	}
	return static_cast<int>( getNumber() );
}

double MixedValue::getDouble() const
{
	if (type == MixedValueType::Double) {
		try {
			auto& result = std::get<double>(value_v);
			return result;
		}
		catch (const std::bad_variant_access& ex) {
		}
	}
	return getNumber();
}

double MixedValue::getNumber() const
{
	double result;
	
	try {
		switch(type) {
			case MixedValueType::Boolean:
			{
				auto& value = std::get<bool>(value_v);
				return ( static_cast<double>(value) );
			}
			case MixedValueType::Int:
			{
				auto& value = std::get<int>(value_v);
				return ( static_cast<double>(value) );
			}
			case MixedValueType::Double:
			{
				auto& value = std::get<double>(value_v);
				return value;
			}
			case MixedValueType::String:
			{
				auto& value = std::get<std::string>(value_v);
				if (STI::Utils::stringToValue(value, result)) {
					return result;
				}
			}
		}
	}
	catch (const std::bad_variant_access& ex) {
	}

	result = 0;
	return (0.0 / result);	//NaN
}

std::string MixedValue::getString() const
{
	try {
		auto& result = std::get<std::string>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	std::string value_s;
	return value_s;
}

const MixedValueVector& MixedValue::getVector() const
{
	try {
		auto& result = std::get<MixedValueVector>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	return empty;
}

std::shared_ptr<STI::Utils::FileHolder> MixedValue::getFile() const
{
	try {
		auto& result = std::get<std::shared_ptr<STI::Utils::FileHolder>>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	std::shared_ptr<STI::Utils::FileHolder> value_file;
	return value_file;
}


std::string MixedValue::print() const
{
	std::stringstream result;
	
	//{ Empty, Boolean, Int, Double, String, Vector, VectorInt, File, Image, Any}

	switch(type)
	{
	case MixedValueType::Empty:
		result << "<Empty>";
		break;
	case MixedValueType::Boolean:
		result << getBoolean();
		break;
	case MixedValueType::Int:
		result << getInt();
		break;
	case MixedValueType::Double:
		result << getDouble();
		break;
	case MixedValueType::String:
		result << getString();
		break;
	case MixedValueType::Vector:
		{
			const auto& values = getVector();
			result << "(";
			for(unsigned i = 0; i < values.size(); ++i)
			{
				if(i > 0)
				{
					result << ",";
				}
				result << values.at(i).print();
			}
			result << ")";
		}
		break;
	case MixedValueType::VectorInt:
		{
			const std::vector<int>* values;
			if (getFlatVector(values)) {
				result << "(";
				for(unsigned i = 0; i < values->size(); ++i)
				{
					if(i > 0)
					{
						result << ",";
					}
					result << values->at(i);
				}
				result << ")";
			}
		}
		break;

	default:
		//this should never happen
		break;
	}

	return result.str();
}


std::string MixedValue::TypeToString(const MixedValueType& type)
{
	//{ Empty, Boolean, Int, Double, String, Vector, File, Image, Any};
	std::string result = "";
	
	switch (type)
	{
	case MixedValueType::Empty:
		result = "Empty";
		break;
	case MixedValueType::Boolean:
		result = "Boolean";
		break;
	case MixedValueType::Int:
		result = "Int";
		break;
	case MixedValueType::Double:
		result = "Double";
		break;
	case MixedValueType::String:
		result = "String";
		break;
	case MixedValueType::Vector:
		result = "Vector";
		break;
	case MixedValueType::VectorInt:
		result = "VectorInt";
		break;
	case MixedValueType::File:
		result = "File";
		break;
	case MixedValueType::Any:
		result = "Any";
		break;
	default:
		//this should never happen
		result = "Unknown";
		break;
	}
	return result;
}

void MixedValue::printError()
{
	std::cout << "Error: Unsupported type was passed to MixedValue." << std::endl;
}


template<class Archive>
void MixedValue::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("type", type), 
		// cereal::make_nvp("value_b", value_b), 
		// cereal::make_nvp("value_i", value_i), 
		// cereal::make_nvp("value_d", value_d), 
		// cereal::make_nvp("value_s", value_s),
		// cereal::make_nvp("value_file", value_file),
		cereal::make_nvp("value_v", value_v)
		// cereal::make_nvp("values", values)
		);
}


template void MixedValue::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void MixedValue::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

