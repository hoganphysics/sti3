
#include "NetworkConvert.h"

#include "orbTypes.h"
#include "MixedValue.h"

using STI::Network::convert;

using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValueType;
using STI::Utils::MixedValueType;


template<>
std::string STI::Network::convert<::CORBA::String_member, std::string>(const ::CORBA::String_member& str)
{
	return std::string(str);
}

template<>
::CORBA::String_member STI::Network::convert<std::string, ::CORBA::String_member>(const std::string& str)
{
	return CORBA::string_dup(str.c_str());
}


template<>
::CORBA::UShort STI::Network::convert<unsigned short, ::CORBA::UShort>(const unsigned short& ushort)
{
	return static_cast<::CORBA::UShort>(ushort);
}


//MixedValue
template<>
TMixedValue STI::Network::convert<MixedValue, TMixedValue>(const MixedValue& value)
{
	TMixedValue tValue;

	switch (value.getType())
	{
	case MixedValueType::Boolean:
		tValue.value_b = static_cast<CORBA::Boolean>(value.getBoolean());
		break;
	case MixedValueType::Int:
		tValue.value_i = static_cast<CORBA::Long>(value.getInt());
		break;
	case MixedValueType::Double:
		tValue.value_d = static_cast<CORBA::Double>(value.getDouble());
		break;
	case MixedValueType::String:
		tValue.value_s = convert<std::string, ::CORBA::String_member>(value.getString());
		break;
	case MixedValueType::Vector:

		convert<MixedValue, TMixedValue>(value.getVector(), tValue.values);

		// const std::vector<MixedValue>& values = value.getVector();
		// tValue.values.length(values.size());
		
		// for (unsigned i = 0; i < values.size(); ++i) {
		// 	tValue.values[i] = convert<MixedValue, TMixedValue>(values.at(i));
		// }

		break;
	case MixedValueType::Empty:
		break;
	case MixedValueType::File:
		break;
	case MixedValueType::Image:
		break;
	case MixedValueType::Any:
		break;
	default:
		break;
	}

	return tValue;
}

template<>
MixedValue STI::Network::convert<TMixedValue, MixedValue>(const TMixedValue& tValue)
{
	MixedValue value;

	switch (tValue.type)
	{
	case TMixedValueType::MixedValueBoolean:
		value = static_cast<bool>(tValue.value_b);
		break;
	case TMixedValueType::MixedValueInt:
		value = static_cast<int>(tValue.value_i);
		break;
	case TMixedValueType::MixedValueDouble:
		value = static_cast<double>(tValue.value_d);
		break;
	case TMixedValueType::MixedValueString:
		value = convert<::CORBA::String_member, std::string>(tValue.value_s);
		break;
	case TMixedValueType::MixedValueVector:

		for (unsigned i = 0; i < tValue.values.length(); ++i) {
			value.addValue(convert<TMixedValue, MixedValue>(tValue.values[i]));
		}

		break;
	case TMixedValueType::MixedValueEmpty:
		break;
	case TMixedValueType::MixedValueFile:
		break;
	case TMixedValueType::MixedValueImage:
		break;
	case TMixedValueType::MixedValueAny:
		break;
	default:
		break;
	}

	return value;
}

template<>
bool STI::Network::convert<MixedValue, TMixedValue>(const MixedValue& value, TMixedValue& tValue)
{
	tValue = convert<MixedValue, TMixedValue>(value);

	return true;
}

template<>
bool STI::Network::convert<TMixedValue, MixedValue>(const TMixedValue& tValue, MixedValue& value)
{
	value = convert<TMixedValue, MixedValue>(tValue);
	return true;
}


