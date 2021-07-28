/*! \file MixedValue.h
 *  \author Jason Michael Hogan
 *  \brief Include-file for the class MixedValue
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

#ifndef STI_UTILS_MIXEDVALUE_H
#define STI_UTILS_MIXEDVALUE_H

#include "fwd/MixedValue_fwd.h"

#include "utils/FileHolder.h"

#include <vector>
#include <string>
#include <memory>

namespace STI
{
namespace Utils
{


class MixedValue
{
public:

	MixedValue();	//for std::vector
	template<typename T> MixedValue(const T& value)
	{
		setValue(value);
	}
	MixedValue(const MixedValue& copy);
	MixedValue(const MixedValueType& value);

	virtual ~MixedValue();

	template<typename T> MixedValue& operator= (const T& other)
	{
		setValue(other);
		return (*this);
	}


	template<typename T> bool operator==(const T& other) const
	{
		return (*this) == MixedValue(other);
	}
	template<typename T> bool operator!=(const T& other) const
	{
		return (*this) != MixedValue(other);
	}


	bool operator==(const MixedValue& other) const;
	bool operator!=(const MixedValue& other) const;

	template<typename T> void setValue(T value)
	{
		//This version of the function is call for all T values that are unsupported.
		//This template is called for all types that don't have an explicitly overloaded setValue function.

		printError();	//temp; push error message
		//std::cout << "Error: Unsupported type was passed to the MixedValue template constructor." << std::endl;
	}

	template<typename T> void setValue(const std::vector<T>& value)
	{
		clear();
		type = MixedValueType::Vector;

		for (unsigned i = 0; i < value.size(); i++)
		{
			addValue(value.at(i));
		}
	}
	void setValue(bool value);
	void setValue(int value);
	void setValue(double value);
	void setValue(const std::shared_ptr<STI::Utils::FileHolder>& value);
	void setValue(const std::string& value);
	void setValue(const MixedValue& value);
	void setValue();	//Empty

	void setValue(const char* value) { setValue(std::string(value)); }
	void setValue(char* value) { setValue(std::string(value)); }

	void setValue(short value) { setValue(static_cast<int>(value)); }
	void setValue(unsigned short value) { setValue(static_cast<int>(value)); }

	void clear();

	template<typename T> void addValue(T value)
	{
		if (type != MixedValueType::Vector)
			convertToVector();

		values.push_back(MixedValue(value));
	}

	MixedValueType getType() const;
	bool isType(const MixedValueType& mixedValueType) const;

	bool getBoolean() const;
	int getInt() const;
	double getDouble() const;
	double getNumber() const;
	std::string getString() const;
	const MixedValueVector& getVector() const;
	std::shared_ptr<STI::Utils::FileHolder> getFile() const;

	std::string print() const;

	static std::string TypeToString(const MixedValueType& type);

	template<class Archive>
	void serialize(Archive& archive);

private:

	void printError();

	void convertToVector();

	MixedValueVector values;

	MixedValueType type;

	bool        value_b;
	int         value_i;
	double      value_d;
	std::string value_s;

	std::shared_ptr<STI::Utils::FileHolder> value_file;

};

} //Utils
} //STI

#endif
