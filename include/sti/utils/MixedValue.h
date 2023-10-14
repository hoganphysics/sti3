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

#include <sti/fwd/MixedValue_fwd.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/FileID.h>

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <ostream>


namespace STI
{
namespace Utils
{

class MixedValue;
class Image;


class MixedValue
{
public:

	MixedValue();	//for std::vector
	// template<typename T> MixedValue(const T& value)
	// {
	// 	std::cout << "template<typename T> MixedValue::MixedValue" << std::endl;
	// 	setValue(value);
	// }
	// template<> MixedValue<MixedValue>(const MixedValue& value)
	// {}
	// template<> void sort(Array<int>&);
	MixedValue(const MixedValue& copy);
	MixedValue(const MixedValueType& value);

	//Template constructor can cause infinite recursion; work around with selection of specific helper constuctors
	MixedValue(bool value);
	MixedValue(int value);
	MixedValue(double value);
	MixedValue(const std::shared_ptr<STI::Utils::BinaryData>& value);
	MixedValue(const FileID& value);
	MixedValue(const std::shared_ptr<STI::Utils::Image>& value);
	MixedValue(const std::string& value);
	MixedValue(const char* value);
	MixedValue(const std::vector<std::string>& values);

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

	std::ostream& operator<<(std::ostream& os);

	template<typename T> void setValue(T value)
	{
		//Catch function for unsupported types.
		printError();	//temp; push error message
	}

	template<typename T> 
	void setValue(const std::vector<T>& value)
	{
		clear();
		convertToVector<T>();

		for (auto& v : value) {
			addValue(v);
		}
	}

	void setValue(bool value);
	void setValue(int value);
	void setValue(double value);
	void setValue(const std::shared_ptr<STI::Utils::BinaryData>& value);
	void setValue(const FileID& value);
	void setValue(const std::shared_ptr<STI::Utils::Image>& value);
	void setValue(const std::string& value);
	void setValue(const MixedValue& value);
	void setValue(const std::vector<std::string>& values);
	void setValue();	//Empty

	void setValue(const char* value) { setValue(std::string(value)); }
	void setValue(char* value) { setValue(std::string(value)); }

	void setValue(short value) { setValue(static_cast<int>(value)); }
	void setValue(unsigned short value) { setValue(static_cast<int>(value)); }
	void setValue(unsigned value) { setValue(static_cast<int>(value)); }
	void setValue(size_t value) { setValue(static_cast<int>(value)); }

	void clear();

	template<typename T> 
	void addValue(const T& value)
	{
		convertToVector<MixedValue>();
		
		try {
			auto& values = std::get<MixedValueVector>(value_v);
			values.push_back(MixedValue());		//empty
			values.back().setValue(value);
		}
		catch (const std::bad_variant_access&) {
		}
	}

	void addValue(const MixedValue& value);
	void addValue(const int& value);

	template<typename T>
	void addValue(const std::string& label, const T& value)
	{
		//add labeled value pair {label, value}
		MixedValue pair;
		pair.addValue(label);
		pair.addValue(value);
		addValue(pair);
	}

	MixedValueType getType() const;
	bool isType(const MixedValueType& mixedValueType) const;
	bool isType(const std::vector<MixedValueType>& types) const;
	bool isNumber() const;
	

	bool getBoolean() const;
	int getInt() const;
	double getDouble() const;
	double getNumber() const;
	std::string getString() const;
	const MixedValueVector& getVector() const;
	MixedValueVector& vec();
	std::shared_ptr<STI::Utils::BinaryData> getBinary() const;
	FileID getFileID() const;
	std::shared_ptr<STI::Utils::Image> getImage() const;

	template<typename T> 
	bool getFlatVector(const std::vector<T>*& flatVector) const
	{
		try {
			auto& vec = std::get<std::vector<T>>(value_v);
			flatVector = &vec;
			return (flatVector != 0);
		}
		catch (const std::bad_variant_access& ex) {
		}
		return false;
	}

	std::string print() const;

	static std::string TypeToString(const MixedValueType& type);

	void swap(MixedValue& value);

	//template<class Archive>
	//void serialize(Archive& archive);

	template<class Archive>
	void save(Archive& archive) const;

	template<class Archive>
	void load(Archive& archive);

private:

	void setValueMixed(const MixedValue& value);

	void printError();

	template<typename T> 
	bool isVectorType()
	{
		return (type == MixedValueType::Vector) 
			|| (type == MixedValueType::VectorInt && std::is_same<int, T>());
	}

	template<typename T> 
	void convertToVector()
	{
		if (isVectorType<T>()) return;
		
		// MixedValueType oldType = type;
		MixedValue oldValue;	//initally empty
		swap(oldValue);

		clear();

		//attempt to make homogeneous vector
		if (std::is_same<int, T>() && 
			(oldValue.isType(MixedValueType::Int) || oldValue.isType(MixedValueType::Empty))) 
		{
			type = MixedValueType::VectorInt;
			std::vector<int> newValues;
			value_v = newValues;
		}
		else {
			//fall back to heterogeneous vector
			type = MixedValueType::Vector;
			MixedValueVector newValues;
			value_v = newValues;
		}

		if (oldValue.isType(MixedValueType::Empty)) return;

		//add old value as the first element of the vector
		try {
			if (type == MixedValueType::VectorInt) {
				auto& values = std::get<std::vector<int>>(value_v);

				auto& value = std::get<int>(oldValue.value_v);
				values.push_back(value);
			}
			else {
				auto& values = std::get<MixedValueVector>(value_v);
				wrapThenAppend(oldValue, values);
				// values.push_back(MixedValue());		//empty
				// values.back().swap(oldValue);
			}
		}
		catch (const std::bad_variant_access&) {
			// swap(oldValue);		//error; reset value
		}

		// if (isVariantMember<std::vector<T>, VariantType>()) {
		// 	std::vector<T> newValues;
		// 	value_v = newValues;
		// }
		// else {
		// 	type = MixedValueType::Vector;
		// 	MixedValueVector newValues;
		// 	value_v = newValues;			
		// }		
	}

	void wrapThenAppend(const MixedValue& source, MixedValueVector& target);

	// template<typename T, typename VARIANT_T>
	// struct isVariantMember;

	// template<typename T, typename... ALL_T>
	// struct isVariantMember<T, std::variant<ALL_T...>> : public std::disjunction<std::is_same<T, ALL_T>...> {};


	// MixedValueVector values;

	MixedValueType type;

	//MixedValueType { Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Number, Any}
	typedef std::variant<std::monostate, 
						bool, 
						int, 
						double, 
						std::string, 
						MixedValueVector, 
						std::vector<int>,
						std::shared_ptr<STI::Utils::BinaryData>,
						FileID,
						std::shared_ptr<STI::Utils::Image>
						> VariantType;

	VariantType value_v;

	MixedValueVector empty;	//needed to return reference (when value_v doesn't hold a vector)

	// bool        value_b;
	// int         value_i;
	// double      value_d;
	// std::string value_s;

	// std::shared_ptr<STI::Utils::FileHolder> value_file;

};

} //Utils
} //STI

#endif
