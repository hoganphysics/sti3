#include <sti/utils/MixedValue.h>
#include <sti/utils/Image.h>
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
using STI::Utils::FileID;


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

MixedValue::MixedValue(bool value)
{
	setValue(value);
}

MixedValue::MixedValue(int value)
{
	setValue(value);
}

MixedValue::MixedValue(double value)
{
	setValue(value);
}

MixedValue::MixedValue(const std::shared_ptr<STI::Utils::BinaryData>& value)
{
	setValue(value);
}

MixedValue::MixedValue(const FileID& value)
{
	setValue(value);
}

MixedValue::MixedValue(const std::shared_ptr<STI::Utils::Image>& value)
{
	setValue(value);
}

MixedValue::MixedValue(const std::string& value)
{
	setValue(value);
}

MixedValue::MixedValue(const char* value)
{
	setValue(value);
}


MixedValue::MixedValue(const std::vector<std::string>& values)
{
	setValue(values);
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
	case MixedValueType::Binary:
		{
			auto bin = getBinary();
			auto otherBin = other.getBinary();

			if (bin != 0 && otherBin != 0) {
				result = (*bin) == (*otherBin);
			}
		}
		break;

	case MixedValueType::File:
		{
			auto fileID = getFileID();
			auto otherFileID = other.getFileID();

			result = fileID == otherFileID;

		}
		break;
	case MixedValueType::Image:
		{
			auto image = getImage();
			auto otherImage = other.getImage();

			if (image != 0 && otherImage != 0) {
				result = (*image) == (*otherImage);
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

std::ostream& MixedValue::operator<<(std::ostream& os)
{
	return os << print();
}

void MixedValue::swap(MixedValue& other) noexcept
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

void MixedValue::setValue(const std::shared_ptr<STI::Utils::BinaryData>& value)
{
	clear();

	value_v = value;
	type = MixedValueType::Binary;
}

void MixedValue::setValue(const FileID& value)
{
	clear();

	value_v = value;
	type = MixedValueType::File;
}

void MixedValue::setValue(const std::shared_ptr<STI::Utils::Image>& value)
{
	clear();

	value_v = value;
	type = MixedValueType::Image;
}

void MixedValue::setValue(const std::vector<std::string>& values)
{
	clear();

	for (auto& v : values) {
		addValue(v);
	}
}

void MixedValue::setValue(const MixedValue& value)
{
	setValueMixed(value);
}

void MixedValue::setValueMixed(const MixedValue& value)
{
	//{ Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Number, Any}

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
	case MixedValueType::Binary:
		setValue( value.getBinary() );
		break;
	case MixedValueType::File:
		setValue( value.getFileID() );
		break;
	case MixedValueType::Image:
		setValue( value.getImage() );
		break;
	default:
		//this should never happen
		break;
	}

}

void MixedValue::setValue()
{
	clear();
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
	return type == mixedValueType || (mixedValueType == MixedValueType::Number && isNumber());
}

bool MixedValue::isType(const std::vector<MixedValueType>& types) const
{
	if (!isType(MixedValueType::Vector)) return false;

	const MixedValueVector& vecItems = getVector();

	if (types.size() != vecItems.size()) return false;

	bool match = true;

	unsigned i = 0;
	for (auto& t : types) {
		match &= (t == MixedValueType::Any) || vecItems.at(i).isType(t) 
			|| (t == MixedValueType::Number && vecItems.at(i).isNumber());
		i++;
	}
	return match;
}

bool MixedValue::isNumber() const
{
	return isType(MixedValueType::Int) || isType(MixedValueType::Double)
		|| isType(MixedValueType::Boolean) || isType(MixedValueType::Number);
}

bool MixedValue::isEmpty() const 
{
	return type == MixedValueType::Empty;
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

MixedValueVector& MixedValue::vec()
{
	try {
		auto& result = std::get<MixedValueVector>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	return empty;
}

std::shared_ptr<STI::Utils::BinaryData> MixedValue::getBinary() const
{
	try {
		auto& result = std::get<std::shared_ptr<STI::Utils::BinaryData>>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	std::shared_ptr<STI::Utils::BinaryData> value_bin;
	return value_bin;
}

FileID MixedValue::getFileID() const
{
	try {
		auto result = std::get<FileID>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	FileID value_file;
	return value_file;
}

std::shared_ptr<STI::Utils::Image> MixedValue::getImage() const
{
	try {
		auto& result = std::get<std::shared_ptr<STI::Utils::Image>>(value_v);
		return result;
	}
	catch (const std::bad_variant_access& ex) {
	}

	std::shared_ptr<STI::Utils::Image> value_image;
	return value_image;
}

std::string MixedValue::print() const
{
	std::stringstream result;
	
	//{ Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Any}

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
	case MixedValueType::Binary:
		{
			auto bin = getBinary();
			result << "BinaryData(";
			if (bin != 0) {
				result << "size=";
				result << bin->bytes();
			}
			else {
				result << "null";
			}
			result << ")";
		}
		break;
	case MixedValueType::File:
		{
			auto file = getFileID();
			result << "File(";

			result << file.filename;

			result << ")";
		}
		break;
	case MixedValueType::Image:
		{
			auto image = getImage();
			result << "Image(";
			if (image != 0) {
				result << image->getFileID().print();
				result << ", height=" << image->getHeight();
				result << ", width=" << image->getWidth();
			}
			else {
				result << "null";
			}
			result << ")";
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
	//{ Empty, Boolean, Int, Double, String, Vector, File, Image, Number, Any};
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
	case MixedValueType::Binary:
		result = "Binary";
		break;
	case MixedValueType::File:
		result = "File";
		break;
	case MixedValueType::Image:
		result = "Image";
		break;
	case MixedValueType::Number:
		result = "Number";
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
void MixedValue::save(Archive& archive) const
{
	//Save image to file before serialization
	if (isType(MixedValueType::Image)) {
		auto image = getImage();
		std::shared_ptr<Image> newImage;

		if (image != 0) {
			image->saveToFile();
			newImage = std::make_shared<Image>(*image);		//shallow clone
		}
		else {
			//error
			newImage = std::make_shared<Image>();
		}

		MixedValue newValue;
		newValue.setValue(newImage);	//need to replace with newImage to avoid possible polymorphic serialization (if Image is some derived type)

		archive(
			cereal::make_nvp("type", newValue.type),
			cereal::make_nvp("value_v", newValue.value_v)
		);

		return;
	}

	archive(
		cereal::make_nvp("type", type),
		cereal::make_nvp("value_v", value_v)
	);
}

template<class Archive>
void MixedValue::load(Archive& archive)
{
	archive(
		cereal::make_nvp("type", type),
		cereal::make_nvp("value_v", value_v)
	);
}

template void MixedValue::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&) const;
template void MixedValue::save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&) const;

template void MixedValue::load<cereal::XMLInputArchive>(cereal::XMLInputArchive&);
template void MixedValue::load<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
