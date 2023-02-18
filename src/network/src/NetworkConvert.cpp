
#include "NetworkConvert.h"

#include <sti/utils/MixedValue.h>

#include "orbTypes.h"
#include "TFileHolderRefInterface.h"
#include "RemoteFileHolder.h"

#include <vector>

using STI::Network::convert;
using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValueType;
using STI::Utils::MixedValueType;
using STI::TNetwork::TStringPairSeq;
using STI::Utils::BinaryData;
using STI::TNetwork::TBinaryData;


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
bool STI::Network::convert<std::string, ::CORBA::String_member>(const std::string& str, ::CORBA::String_member& tString)
{
	tString = CORBA::string_dup(str.c_str());
	return true;
}


template<>
bool STI::Network::convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(
	const std::vector<std::string>& stringVec, STI::TNetwork::TStringSeq& tStringSeq)
{
	tStringSeq.length(static_cast<unsigned>(stringVec.size()));

	typename std::vector<std::string>::const_iterator in = stringVec.begin();
	for (unsigned i = 0; i < tStringSeq.length() && in != stringVec.end(); ++i, ++in) {
		tStringSeq[i] = convert<std::string, ::CORBA::String_member>(*in);
	}
	return (stringVec.size() == tStringSeq.length());
}

template<>
bool STI::Network::convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(
	const STI::TNetwork::TStringSeq& tStringSeq, std::vector<std::string>& stringVec)
{
	stringVec.clear();
	for (unsigned i = 0; i < tStringSeq.length(); ++i) {
		//stringVec.push_back(convert<::CORBA::String_member, std::string>(tStringSeq[i]));
		stringVec.push_back(tStringSeq[i]._NP_ref());
	}
	return (stringVec.size() == tStringSeq.length());
}


template<>
bool STI::Network::convert<std::map<std::string, std::string>, STI::TNetwork::TStringPairSeq>(
	const std::map<std::string, std::string>& stringMap, STI::TNetwork::TStringPairSeq& tStringMap)
{
	tStringMap.length(static_cast<CORBA::ULong>(stringMap.size()) );

	auto it = stringMap.begin();
	for (unsigned i = 0; it != stringMap.end() && i < tStringMap.length(); ++i, ++it) {
		tStringMap[i].key = convert<std::string, ::CORBA::String_member>(it->first);
		tStringMap[i].value = convert<std::string, ::CORBA::String_member>(it->second);
	}

	return (stringMap.size() == tStringMap.length());
}

template<>
bool STI::Network::convert<STI::TNetwork::TStringPairSeq, std::map<std::string, std::string>>(
	const STI::TNetwork::TStringPairSeq& tStringMap, std::map<std::string, std::string>& stringMap)
{
	stringMap.clear();

	for (unsigned i = 0; i < tStringMap.length(); ++i) {
		stringMap.insert( std::pair<std::string, std::string>(
							convert<::CORBA::String_member, std::string>(tStringMap[i].key),
							convert<::CORBA::String_member, std::string>(tStringMap[i].value)
						));
	}
	return (stringMap.size() == tStringMap.length());
}




// Buffer

// template<>
// bool STI::Network::convert<char*, ::STI::TNetwork::OctetSeq>(const char*& buffer, ::STI::TNetwork::OctetSeq& tBuffer)
// {
// 	return false;
// }

bool STI::Network::convertBuffer(const char* buffer, unsigned length, ::STI::TNetwork::OctetSeq& tBuffer)
{
	tBuffer.length(length);
	
	for (unsigned i = 0; i < tBuffer.length(); ++i) {
		tBuffer[i] = buffer[i];
	}
	return false;
}

bool STI::Network::convertBuffer(const STI::TNetwork::OctetSeq& tBuffer, char* buffer)
{
	for (unsigned i = 0; i < tBuffer.length(); ++i) {
		buffer[i] = tBuffer[i];
	}
	return true;
}

// template<>
// bool STI::Network::convert<::STI::TNetwork::OctetSeq, char*>(const STI::TNetwork::OctetSeq& tBuffer, char*& buffer)
// {
// 	for (unsigned i = 0; i < tBuffer.length(); ++i) {
// 		buffer[i] = tBuffer[i];
// 	}
// 	return true;
// }


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
	convert<MixedValue, TMixedValue>(value, tValue);
	return tValue;
}


template<>
bool STI::Network::convert<MixedValue, TMixedValue>(const MixedValue& value, TMixedValue& tValue)
{
	// tValue.type = convert<MixedValueType, TMixedValueType>(value.getType());

	switch (value.getType())
	{
	case MixedValueType::Empty:
		tValue.empty();
		break;
	case MixedValueType::Boolean:
		tValue.value_b( static_cast<CORBA::Boolean>(value.getBoolean()) );
		break;
	case MixedValueType::Int:
		tValue.value_i( static_cast<CORBA::Long>(value.getInt()) );
		break;
	case MixedValueType::Double:
		tValue.value_d( static_cast<CORBA::Double>(value.getDouble()) );
		break;
	case MixedValueType::String:
		tValue.value_s( convert<std::string, ::CORBA::String_member>(value.getString()) );
		break;
	case MixedValueType::Vector:
		tValue.values(STI::TNetwork::TMixedValueSeq());
		convert<MixedValue, TMixedValue>(value.getVector(), tValue.values());

		// const std::vector<MixedValue>& values = value.getVector();
		// tValue.values.length(values.size());
		
		// for (unsigned i = 0; i < values.size(); ++i) {
		// 	tValue.values[i] = convert<MixedValue, TMixedValue>(values.at(i));
		// }

		break;
	case MixedValueType::VectorInt:
		{
			const std::vector<int>* values;
			if (!value.getFlatVector(values)) return false;

			tValue.valuesInt(STI::TNetwork::TMixedValue::_valuesInt_seq());
			auto& valuesIntRef = tValue.valuesInt();

			//Hack here. Striping const qualifer away from input value to get access to vector raw data.
			//The TMixedValue type is given a reference to the memory location of the input vector (requires non const).
			//This is potentially dangerous if the TMixedValue is later modifed. However, we know that convert<> is only being
			//called in order to marshal and then transmit the data over the network.
			//So we avoid an unnecessary deep copy by sharing a raw pointer to the data.
			//Here replace() sets the pointer in the underlying corba sequence to raw data. We use release_=false when
			//calling replace() to ensure that the corba sequence will not attempt to free this memory, which would
			//cause a double free.
			valuesIntRef.replace(values->size(), values->size(), const_cast<std::vector<int>*>(values)->data(), false);		//no release
		}
		break;
	case MixedValueType::Binary:
		{
			auto bin = value.getBinary();

			if (bin != 0 ) {
				convert<BinaryData, TBinaryData>(*bin, tValue.valueBin());

				// valueBinRef.wordsize = static_cast<CORBA::Short>(bin->wordsize());

				// unsigned char* data;
				// int* dataInt;

				// if (bin->isType<unsigned char>() && bin->get(data)) {
				// 	valueBinRef.data.data_char().replace(bin->bytes(), bin->bytes(), data, false );	//no release
				// }
				// else if (bin->isType<int>() && bin->get(dataInt)) {
				// 	// data = reinterpret_cast<unsigned char*>(dataInt);
				// 	valueBinRef.data.data_long().replace(bin->bytes(), bin->bytes(), dataInt, false );	//no release
				// }
			}
		}
		break;
	case MixedValueType::File:
		{
			STI::TNetwork::TFileHolder_var tFileHolder;
			TFileHolderRefInterface::getTFileHolderReference(value.getFile(), tFileHolder);
			tValue.value_file(tFileHolder);			
		}
		break;
	case MixedValueType::Image:
		break;
	case MixedValueType::Any:
		break;
	default:
		break;
	}

	return true;
}

template<>
MixedValue STI::Network::convert<TMixedValue, MixedValue>(const TMixedValue& tValue)
{
	MixedValue value;

	convert<TMixedValue, MixedValue>(tValue, value);

	return value;
}

// template<>
// bool STI::Network::convert<MixedValue, TMixedValue>(const MixedValue& value, TMixedValue& tValue)
// {
// 	tValue = convert<MixedValue, TMixedValue>(value);

// 	return true;
// }

template<>
bool STI::Network::convert<TMixedValue, MixedValue>(const TMixedValue& tValue, MixedValue& value)
{
	// value = convert<TMixedValue, MixedValue>(tValue);

	switch (tValue._d())
	{
	case TMixedValueType::MixedValueEmpty:
		value.clear();
		break;
	case TMixedValueType::MixedValueBoolean:
		value = static_cast<bool>(tValue.value_b());
		break;
	case TMixedValueType::MixedValueInt:
		value = static_cast<int>(tValue.value_i());
		break;
	case TMixedValueType::MixedValueDouble:
		value = static_cast<double>(tValue.value_d());
		break;
	case TMixedValueType::MixedValueString:
		value = convert<::CORBA::String_member, std::string>(tValue.value_s());
		break;
	case TMixedValueType::MixedValueVector:

		for (unsigned i = 0; i < tValue.values().length(); ++i) {
			value.addValue(convert<TMixedValue, MixedValue>(tValue.values()[i]));
		}

		break;
	case TMixedValueType::MixedValueVectorInt:
		{
			std::vector<int> newVec;
			value.setValue(newVec);		//convert to VectorInt

			const std::vector<int>* values;
			if (value.getFlatVector(values)) {
				// std::vector<int> newVec2;
				std::vector<int>* pValues = const_cast<std::vector<int>*>(values);

				//For now, do this the slow way (deep copy)
				pValues->resize(tValue.valuesInt().length());
				auto& tValues = tValue.valuesInt();
				for (unsigned i = 0; i < tValues.length() && i < pValues->size(); ++i) {
					(*pValues)[i] = tValues[i];
				}
				// const_cast<std::vector<int>*>(values)->data() = (newVec2.data());
			}

			//Ideas for a more efficient way, avoiding copies:
			// bool release = tValue.valuesInt().release();
			// int* buffer = const_cast<TMixedValue&>(tValue).valuesInt().get_buffer( release );	//orphan if release = true
			//The data stored in buffer is no longer managed, so we must delete it ourselves (or hand to a vector which will delete...)
			//Conceptually we now want:   values->data() = buffer;	but this is not possible in portable way...
		}
		break;
	case TMixedValueType::MixedValueBinary:
		{
			auto bin = std::make_shared<BinaryData>();
			value.setValue(bin);

			if (bin != 0 ) {
				convert<TBinaryData, BinaryData>(tValue.valueBin(), *bin);
			}
		}
		break;
	case TMixedValueType::MixedValueFile:
		{
			std::shared_ptr<STI::Utils::FileHolder> remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(tValue.value_file());
			value.setValue(remoteFile);	
		}
		break;
	case TMixedValueType::MixedValueImage:
		break;
	case TMixedValueType::MixedValueAny:
		break;
	default:
		break;
	}

	return true;
}



template<>
bool STI::Network::convert<BinaryData, TBinaryData>(const BinaryData& bin, TBinaryData& tBin)
{
	tBin.wordsize = static_cast<CORBA::Short>(bin.wordsize());

	//Here replace() sets the pointer in the underlying corba sequence to raw data. We use release_=false when
	//calling replace() to ensure that the corba sequence will not attempt to free this memory,
	//since BinaryData is the owner.

	if (bin.isType<unsigned char>()) {
		unsigned char* data;
		if (bin.get(data)) {
			tBin.data.data_uchar().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<signed char>()) {
		signed char* data;
		if (bin.get(data)) {
			unsigned char* dataUC = reinterpret_cast<unsigned char*>(data);
			tBin.data.data_char().replace(bin.length(), bin.length(), dataUC, false );	//no release
		}
	}
	else if (bin.isType<char>()) {
		char* data;
		if (bin.get(data)) {
			unsigned char* dataUC = reinterpret_cast<unsigned char*>(data);
			tBin.data.data_char().replace(bin.length(), bin.length(), dataUC, false );	//no release
		}
	}
	else if (bin.isType<unsigned short>()) {
		unsigned short* data;
		if (bin.get(data)) {
			tBin.data.data_ushort().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<short>()) {
		short* data;
		if (bin.get(data)) {
			tBin.data.data_short().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<unsigned int>()) {
		unsigned int* data;
		if (bin.get(data)) {
			tBin.data.data_ulong().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<int>()) {
		int* data;
		if (bin.get(data)) {
			tBin.data.data_long().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<float>()) {
		float* data;
		if (bin.get(data)) {
			tBin.data.data_float().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	else if (bin.isType<double>()) {
		double* data;
		if (bin.get(data)) {
			tBin.data.data_double().replace(bin.length(), bin.length(), data, false );	//no release
		}
	}
	return true;
}

template<>
bool STI::Network::convert<TBinaryData, BinaryData>(const TBinaryData& tBin, BinaryData& bin)
{
	//TBinaryType { BinaryUChar, BinaryChar, BinaryUShort, BinaryShort, BinaryULong, BinaryLong, BinaryFloat, BinaryDouble };
	using STI::TNetwork::TBinaryType;

	bin.clear();
	bool release;

	switch (tBin.data._d())
	{
	case TBinaryType::BinaryUChar:
		{
			release = tBin.data.data_uchar().release();
			unsigned char* data = const_cast<TBinaryData&>(tBin).data.data_uchar().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_char().length());
		}
		break;
	case TBinaryType::BinaryChar:
		{
			release = tBin.data.data_char().release();
			unsigned char* data = const_cast<TBinaryData&>(tBin).data.data_char().get_buffer(release);	//orphan if release = true
			char* dataChar = reinterpret_cast<char*>(data);
			bin.assign(dataChar, tBin.data.data_char().length());
		}
		break;
	case TBinaryType::BinaryUShort:
		{
			release = tBin.data.data_ushort().release();
			unsigned short* data = const_cast<TBinaryData&>(tBin).data.data_ushort().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_ushort().length());
		}
		break;
	case TBinaryType::BinaryShort:
		{
			release = tBin.data.data_short().release();
			short* data = const_cast<TBinaryData&>(tBin).data.data_short().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_short().length());
		}
		break;
	case TBinaryType::BinaryULong:
		{
			CORBA::ULong x;
			release = tBin.data.data_ulong().release();
			unsigned int* data = const_cast<TBinaryData&>(tBin).data.data_ulong().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_ulong().length());
		}
		break;
	case TBinaryType::BinaryLong:
		{
			release = tBin.data.data_long().release();
			int* data = const_cast<TBinaryData&>(tBin).data.data_long().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_long().length());
		}
		break;
	case TBinaryType::BinaryFloat:
		{
			release = tBin.data.data_float().release();
			float* data = const_cast<TBinaryData&>(tBin).data.data_float().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_float().length());
		}
		break;
	case TBinaryType::BinaryDouble:
		{
			release = tBin.data.data_double().release();
			double* data = const_cast<TBinaryData&>(tBin).data.data_double().get_buffer(release);	//orphan if release = true
			bin.assign(data, tBin.data.data_double().length());
		}
		break;
	default:
		break;
	}

	return true;
}



template<>
TMixedValueType STI::Network::convert<MixedValueType, TMixedValueType>(const MixedValueType& type)
{
	TMixedValueType tType;

	switch (type)
	{
	case MixedValueType::Empty:
		tType = TMixedValueType::MixedValueEmpty;
		break;
	case MixedValueType::Boolean:
		tType = TMixedValueType::MixedValueBoolean;
		break;
	case MixedValueType::Int:
		tType = TMixedValueType::MixedValueInt;
		break;
	case MixedValueType::Double:
		tType = TMixedValueType::MixedValueDouble;
		break;
	case MixedValueType::String:
		tType = TMixedValueType::MixedValueString;
		break;
	case MixedValueType::Vector:
		tType = TMixedValueType::MixedValueVector;
		break;
	case MixedValueType::VectorInt:
		tType = TMixedValueType::MixedValueVectorInt;
		break;
	case MixedValueType::File:
		tType = TMixedValueType::MixedValueFile;
		break;
	case MixedValueType::Image:
		tType = TMixedValueType::MixedValueImage;
		break;
	case MixedValueType::Any:
		tType = TMixedValueType::MixedValueAny;
		break;
	default:
		tType = TMixedValueType::MixedValueAny;
		break;
	}

	return tType;

}

template<>
MixedValueType STI::Network::convert<TMixedValueType, MixedValueType>(const TMixedValueType& tType)
{
	MixedValueType type;

	switch (tType)
	{
	case TMixedValueType::MixedValueEmpty:
		type = MixedValueType::Empty;
		break;
	case TMixedValueType::MixedValueBoolean:
		type = MixedValueType::Boolean;
		break;
	case TMixedValueType::MixedValueInt:
		type = MixedValueType::Int;
		break;
	case TMixedValueType::MixedValueDouble:
		type = MixedValueType::Double;
		break;
	case TMixedValueType::MixedValueString:
		type = MixedValueType::String;
		break;
	case TMixedValueType::MixedValueVector:
		type = MixedValueType::Vector;
		break;
	case TMixedValueType::MixedValueVectorInt:
		type = MixedValueType::VectorInt;
		break;
	case TMixedValueType::MixedValueFile:
		type = MixedValueType::File;
		break;
	case TMixedValueType::MixedValueImage:
		type = MixedValueType::Image;
		break;
	case TMixedValueType::MixedValueAny:
		type = MixedValueType::Any;
		break;
	default:	
		type = MixedValueType::Any;
		break;
	}

	return type;
}


//GraphPathLabel
bool STI::Network::convertEventGraphPath(const STI::Utils::GraphPathLabel& graphPath, ::STI::TNetwork::TGraphPathLabel& tGraphPath)
{
    tGraphPath.length(static_cast<CORBA::ULong>(graphPath.size()));

    for (unsigned i = 0; i < graphPath.size(); ++i) {
        tGraphPath[i] = static_cast<CORBA::ULong>(graphPath.at(i));
    }
    return true;
}

bool STI::Network::convertEventGraphPath(const ::STI::TNetwork::TGraphPathLabel& tGraphPath, STI::Utils::GraphPathLabel& graphPath)
{
    for (unsigned i = 0; i < tGraphPath.length(); ++i) {
         graphPath.push_back( static_cast<unsigned>(tGraphPath[i]) );
    }

    return true;
}
