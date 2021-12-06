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


#include "MixedValue.h"
#include "utils.h"

#include "CerealArchives.h"

#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <sstream>
#include <iostream>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueVector;
using STI::Utils::MixedValueType;

MixedValue::MixedValue()
{
	type = MixedValueType::Empty;
}
MixedValue::MixedValue(const MixedValue& copy)
{
	setValue(copy);
}

MixedValue::MixedValue(const MixedValueType& valueType)
{
	if (valueType == MixedValueType::Empty) {
		type = MixedValueType::Empty;
	}
	else {
		setValue(valueType);	//error
	}
}

MixedValue::~MixedValue()
{
}


bool MixedValue::operator==(const MixedValue& other) const
{
	if(type != other.getType())
		return false;

	bool result;

	switch(type)
	{
	case MixedValueType::Boolean:
		result = ( value_b == other.getBoolean() );
		break;
	case MixedValueType::Int:
		result = ( value_i == other.getInt() );
		break;
	case MixedValueType::Double:
		result = ( value_d == other.getDouble() );
		break;
	case MixedValueType::String:
		result = ( value_s.compare(other.getString()) == 0 );
		break;
	case MixedValueType::Vector:
		{
			const MixedValueVector& otherValues = other.getVector();

			if(values.size() != otherValues.size())
			{
				result = false;
			}
			else
			{
				result = true;

				for(unsigned i = 0; (i < otherValues.size() && i < values.size()); i++)
				{
					result &= ( values.at(i) == otherValues.at(i) );
				}
			}
		}
		break;
	case MixedValueType::Empty:
		return true;	//both are empty
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

void MixedValue::setValue(bool value)
{
	clear();

	value_b = value;
	type = MixedValueType::Boolean;
}

void MixedValue::setValue(int value)
{
	clear();

	value_i = value;
	type = MixedValueType::Int;
}


void MixedValue::setValue(double value)
{
	clear();

	value_d = value;
	type = MixedValueType::Double;
}

void MixedValue::setValue(const std::string& value)
{
	clear();

	value_s = value;
	type = MixedValueType::String;
}

void MixedValue::setValue(const std::shared_ptr<STI::Utils::FileHolder>& value)
{
	clear();

	value_file = value;
	type = MixedValueType::File;
}

void MixedValue::setValue(const MixedValue& value)
{
	//clear();
	//type = value.getType();

	switch( value.getType() )
	{
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
	case MixedValueType::Empty:
		setValue();
		break;
	default:
		//this should never happen
		break;
	}

}

void MixedValue::setValue()
{
	clear();
	type = MixedValueType::Empty;
}

void MixedValue::clear()
{
	values.clear();
	type = MixedValueType::Vector;
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
	if(type == MixedValueType::Boolean)
		return value_b;
	else
		return (getNumber() != 0);
}

int MixedValue::getInt() const
{
	if(type == MixedValueType::Int)
		return value_i;
	else
		return static_cast<int>( getNumber() );
}

double MixedValue::getDouble() const
{
	if(type == MixedValueType::Double)
		return value_d;
	else
		return static_cast<double>( getNumber() );
}

double MixedValue::getNumber() const
{
	double result;
	
	switch(type)
	{
	case MixedValueType::Boolean:
		return ( static_cast<double>(value_b) );
	case MixedValueType::Int:
		return ( static_cast<double>(value_i) );
	case MixedValueType::Double:
		return value_d;
	case MixedValueType::String:
		if(STI::Utils::stringToValue(value_s, result))
			return result;
		else
		{
			result = 0;
			return (0.0 / result);	//NaN
		}
	default:
		result = 0;
		return (0.0 / result);	//NaN
	}
}

std::string MixedValue::getString() const
{
	return value_s;
}

const MixedValueVector& MixedValue::getVector() const
{
	return values;
}

std::shared_ptr<STI::Utils::FileHolder> MixedValue::getFile() const
{
	return value_file;
}

void MixedValue::convertToVector()
{
	if(type == MixedValueType::Vector)
		return;

	MixedValueType oldType = type;

	clear();
	type = MixedValueType::Vector;

	switch(oldType)
	{
	case MixedValueType::Boolean:
		addValue(value_b);
		break;
	case MixedValueType::Int:
		addValue(value_i);
		break;
	case MixedValueType::Double:
		addValue(value_d);
		break;
	case MixedValueType::String:
		addValue(value_s);
		break;
	case MixedValueType::Empty:
		break;
	default:
		//this should never happen
		break;
	}
}

std::string MixedValue::print() const
{
	std::stringstream result;
	
	switch(type)
	{
	case MixedValueType::Boolean:
		result << value_b;
		break;
	case MixedValueType::Int:
		result << value_i;
		break;
	case MixedValueType::Double:
		result << value_d;
		break;
	case MixedValueType::String:
		result << value_s;
		break;
	case MixedValueType::Vector:
		result << "(";
		for(unsigned i = 0; i < values.size(); i++)
		{
			if(i > 0)
			{
				result << ",";
			}
			result << values.at(i).print();
		}
		result << ")";
		break;
	case MixedValueType::Empty:
		result << "<Empty>";
		break;
	default:
		//this should never happen
		break;
	}

	return result.str();
}


std::string MixedValue::TypeToString(const MixedValueType& type)
{
	//{ Boolean, Int, Double, String, Vector, Empty, File, Image, Any };
	std::string result = "";
	
	switch (type)
	{
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
	case MixedValueType::Empty:
		result = "Empty";
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
	std::cout << "Error: Unsupported type was passed to the MixedValue template constructor." << std::endl;
}


template<class Archive>
void MixedValue::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("type", type), 
		cereal::make_nvp("value_b", value_b), 
		cereal::make_nvp("value_i", value_i), 
		cereal::make_nvp("value_d", value_d), 
		cereal::make_nvp("value_s", value_s),
		cereal::make_nvp("value_file", value_file),
		cereal::make_nvp("values", values)
		);
}


template void MixedValue::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void MixedValue::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

