
#include "NetworkConvert.h"

#include <sti/utils/MixedValue.h>
#include <sti/utils/Image.h>

#include "convert/Convert_File.h"

#include "TFileHolderRefInterface.h"
#include "RemoteFileHolder.h"
#include "RemoteBinaryDataStream.h"
#include "NetworkBinaryDataStreamTarget.h"
#include "NetworkBinaryDataStream.h"

#include "generated/orbTypes.h"

#include <omniORB4/internal/orbParameters.h>
#include <algorithm>
#include <limits>
#include <vector>


using STI::Network::convert;
using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValueType;
using STI::Utils::MixedValueType;
using STI::TNetwork::TStringPairSeq;
using STI::Utils::BinaryData;
using STI::TNetwork::TBinaryData;
using STI::Utils::Image;
using STI::TNetwork::TImage;
using STI::TNetwork::TImageDataType;
using STI::Utils::FileHolder;
using STI::Utils::FileID;
using STI::TNetwork::TFileID;
using STI::Network::RemoteBinaryDataStream;
using STI::Network::NetworkBinaryDataStreamTarget;
using STI::Network::NetworkBinaryDataStream;

namespace
{
CORBA::ULong toCorbaULong(size_t value)
{
	return static_cast<CORBA::ULong>(
		std::min(value, static_cast<size_t>(std::numeric_limits<CORBA::ULong>::max())));
}

size_t binaryTypeWordsize(STI::TNetwork::TBinaryType type)
{
	using STI::TNetwork::TBinaryType;
	switch (type)
	{
	case TBinaryType::BinaryUShort:
	case TBinaryType::BinaryShort:
		return sizeof(CORBA::Short);
	case TBinaryType::BinaryULong:
	case TBinaryType::BinaryLong:
		return sizeof(CORBA::Long);
	case TBinaryType::BinaryFloat:
		return sizeof(CORBA::Float);
	case TBinaryType::BinaryDouble:
		return sizeof(CORBA::Double);
	case TBinaryType::BinaryUChar:
	case TBinaryType::BinaryChar:
	case TBinaryType::BinaryStream:
	default:
		return sizeof(char);
	}
}

size_t sequenceLength(const TBinaryData& tBin)
{
	using STI::TNetwork::TBinaryType;
	switch (tBin.data._d())
	{
	case TBinaryType::BinaryUChar:
		return tBin.data.data_uchar().length();
	case TBinaryType::BinaryChar:
		return tBin.data.data_char().length();
	case TBinaryType::BinaryUShort:
		return tBin.data.data_ushort().length();
	case TBinaryType::BinaryShort:
		return tBin.data.data_short().length();
	case TBinaryType::BinaryULong:
		return tBin.data.data_ulong().length();
	case TBinaryType::BinaryLong:
		return tBin.data.data_long().length();
	case TBinaryType::BinaryFloat:
		return tBin.data.data_float().length();
	case TBinaryType::BinaryDouble:
		return tBin.data.data_double().length();
	case TBinaryType::BinaryStream:
	default:
		return 0;
	}
}

size_t binaryLengthMetadata(const TBinaryData& tBin)
{
	if (tBin.length != 0 || tBin.bytes == 0) {
		return static_cast<size_t>(tBin.length);
	}
	return sequenceLength(tBin);
}

size_t binaryWordsizeMetadata(const TBinaryData& tBin)
{
	if (tBin.wordsize > 0) {
		return static_cast<size_t>(tBin.wordsize);
	}
	return binaryTypeWordsize(tBin.data._d());
}

void setBinaryMetadata(const std::shared_ptr<BinaryData>& bin, const TBinaryData& tBin)
{
	if (bin == 0) {
		return;
	}

	bin->setMetadata(binaryLengthMetadata(tBin), binaryWordsizeMetadata(tBin));
}

bool getExistingBinaryStreamReference(const std::shared_ptr<BinaryData>& bin,
	STI::TNetwork::TBinaryDataStream_var& tDataStream)
{
	if (bin == 0) {
		return false;
	}

	auto stream = bin->getStream();
	if (stream == 0) {
		return false;
	}

	return RemoteBinaryDataStream::getTBinaryDataStreamRef(stream, tDataStream)
		|| NetworkBinaryDataStream::getTBinaryDataStreamRef(stream, tDataStream);
}

template<typename T>
T* copyBuffer(const T* data, size_t length)
{
	auto* copy = new T[length];
	std::copy(data, data + length, copy);
	return copy;
}

template<typename OutT, typename InT>
OutT* copyBufferAs(const InT* data, size_t length)
{
	auto* copy = new OutT[length];
	for (size_t i = 0; i < length; ++i) {
		copy[i] = static_cast<OutT>(data[i]);
	}
	return copy;
}
} // namespace


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
	//tBuffer.length(length);
	//
	//for (unsigned i = 0; i < tBuffer.length(); ++i) {
	//	tBuffer[i] = buffer[i];
	//}
	//return true;

	// //do not call tBuffer.length(length); This is done by replace.
	unsigned char* data = reinterpret_cast<unsigned char*>(const_cast<char*>(buffer));
	tBuffer.replace(length, length, data, false);	//no release
	return true;

}

bool STI::Network::convertBuffer(const STI::TNetwork::OctetSeq& tBuffer, const char*& buffer)
{
	unsigned char* data = const_cast<STI::TNetwork::OctetSeq&>(tBuffer).get_buffer();	//no deep copy
	buffer = reinterpret_cast<const char*>(data);
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
	return convertMixedValue(value, tValue, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertMixedValue(
	const MixedValue& value,
	TMixedValue& tValue,
	BinaryPayloadPolicy policy)
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
		{
			const auto& values = value.getVector();
			auto& tValues = tValue.values();
			tValues.length(static_cast<CORBA::ULong>(values.size()));

			for (CORBA::ULong i = 0; i < tValues.length() && i < values.size(); ++i) {
				convertMixedValue(values.at(i), tValues[i], policy);
			}
		}

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
			//valuesIntRef.replace(values->size(), values->size(), const_cast<std::vector<int>*>(values)->data(), false);		//no release
			
			valuesIntRef.replace(values->size(), values->size(), 
				reinterpret_cast<CORBA::Long*>( const_cast<std::vector<int>*>(values)->data() )
				, false);		//no release
		}
		break;
	case MixedValueType::Binary:
		{
			auto bin = value.getBinary();

			if (bin != 0 ) {
				TBinaryData tBin;
				convertBinaryData(bin, tBin, policy);
				tValue.valueBin(tBin);

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
			// STI::TNetwork::TFileHolder_var tFileHolder;
			// TFileHolderRefInterface::getTFileHolderReference(value.getFile(), tFileHolder);
			tValue.value_file(convert<FileID, TFileID>(value.getFileID()));
		}
		break;
	case MixedValueType::Image:
		{
			STI::TNetwork::TImage tImage;

			if (value.getImage() != 0) {
				convertImage(*(value.getImage()), tImage, policy);
			}
			
			tValue.value_image(tImage);
		}
		break;
	case MixedValueType::Number:
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
	return convertMixedValue(tValue, value, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertMixedValue(
	const TMixedValue& tValue,
	MixedValue& value,
	BinaryPayloadPolicy policy)
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
			MixedValue child;
			convertMixedValue(tValue.values()[i], child, policy);
			value.addValue(child);
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
				convertBinaryData(tValue.valueBin(), bin, policy);
			}
		}
		break;
	case TMixedValueType::MixedValueFile:
		{
			// std::shared_ptr<STI::Utils::FileHolder> remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(tValue.value_file());
			// value.setValue(remoteFile);
			value.setValue(convert<TFileID, FileID>(tValue.value_file()));
		}
		break;
	case TMixedValueType::MixedValueImage:
		{
			auto image = std::make_shared<Image>();
			convertImage(tValue.value_image(), *image, policy);
			value.setValue(image);
		}
		break;
	case TMixedValueType::MixedValueAny:
		break;
	default:
		break;
	}

	return true;
}

bool STI::Network::convertChannelUpdateMap(
	const std::map<short, MixedValue>& values,
	STI::TNetwork::TChannelUpdateTupleSeq& tValues,
	BinaryPayloadPolicy policy)
{
	tValues.length(static_cast<CORBA::ULong>(values.size()));

	auto it = values.begin();
	for (CORBA::ULong i = 0; i < tValues.length() && it != values.end(); ++i, ++it) {
		tValues[i].channelNumber = static_cast<CORBA::Short>(it->first);
		convertMixedValue(it->second, tValues[i].value, policy);
	}

	return values.size() == tValues.length();
}

bool STI::Network::convertChannelUpdateMap(
	const STI::TNetwork::TChannelUpdateTupleSeq& tValues,
	std::map<short, MixedValue>& values,
	BinaryPayloadPolicy policy)
{
	values.clear();

	for (CORBA::ULong i = 0; i < tValues.length(); ++i) {
		MixedValue value;
		convertMixedValue(tValues[i].value, value, policy);
		values[static_cast<short>(tValues[i].channelNumber)] = value;
	}

	return values.size() == tValues.length();
}

template<>
bool STI::Network::convert<std::shared_ptr<BinaryData>, TBinaryData>(const std::shared_ptr<BinaryData>& bin, TBinaryData& tBin)
{
	return convertBinaryData(bin, tBin, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertBinaryData(
	const std::shared_ptr<BinaryData>& bin,
	TBinaryData& tBin,
	BinaryPayloadPolicy policy)
{
	if (bin == 0) return false;

	bool success = false;

	tBin.wordsize = static_cast<CORBA::Short>(bin->wordsize());
	tBin.length = toCorbaULong(bin->length());
	tBin.bytes = toCorbaULong(bin->bytes());

	size_t maxNetworkMessage = omni::orbParameters::giopMaxMsgSize;

	//size_t maxNetworkMessage = 1000000;

	if (policy == BinaryPayloadPolicy::PreferStreamReference || bin->bytes() > maxNetworkMessage) {
		STI::TNetwork::TBinaryDataStream_var tDataStream;

		if (!bin->hasLocalData() && getExistingBinaryStreamReference(bin, tDataStream)) {
			tBin.data.data_stream(tDataStream);
			return true;
		}

		auto sourceStream = bin->hasLocalData() ? nullptr : bin->getStream();
		auto networkDataStream = sourceStream != 0
			? std::make_shared<NetworkBinaryDataStream>(sourceStream)
			: std::make_shared<NetworkBinaryDataStream>(bin, maxNetworkMessage);
		bin->retainStream(networkDataStream);

		if (!NetworkBinaryDataStream::getTBinaryDataStreamRef(networkDataStream, tDataStream)) {
			return false;
		}
		tBin.data.data_stream(tDataStream);
		return true;
	}

	if (!bin->hasLocalData() && bin->hasStream()) {
		if (!bin->materialize()) {
			return false;
		}
		tBin.wordsize = static_cast<CORBA::Short>(bin->wordsize());
		tBin.length = toCorbaULong(bin->length());
		tBin.bytes = toCorbaULong(bin->bytes());
	}

	//Here replace() sets the pointer in the underlying corba sequence to raw data. We use release_=false when
	//calling replace() to ensure that the corba sequence will not attempt to free this memory,
	//since BinaryData is the owner.

	if (bin->isType<unsigned char*>()) {
		unsigned char* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_uchar_seq();
			tBin.data.data_uchar(seq);
			tBin.data.data_uchar().replace(bin->length(), bin->length(), data, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<signed char*>()) {
		signed char* data;
		if (bin->get(data)) {
			unsigned char* dataUC = reinterpret_cast<unsigned char*>(data);
			auto seq = STI::TNetwork::TMixedBinaryData::_data_char_seq();
			tBin.data.data_char(seq);
			tBin.data.data_char().replace(bin->length(), bin->length(), dataUC, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<char*>()) {
		char* data;
		if (bin->get(data)) {
			unsigned char* dataUC = reinterpret_cast<unsigned char*>(data);
			auto seq = STI::TNetwork::TMixedBinaryData::_data_char_seq();
			tBin.data.data_char(seq);
			tBin.data.data_char().replace(bin->length(), bin->length(), dataUC, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<unsigned short*>()) {
		unsigned short* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_ushort_seq();
			tBin.data.data_ushort(seq);
			tBin.data.data_ushort().replace(bin->length(), bin->length(), data, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<short*>()) {
		short* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_short_seq();
			tBin.data.data_short(seq);
			tBin.data.data_short().replace(bin->length(), bin->length(), data, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<unsigned int*>()) {
		unsigned int* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_ulong_seq();
			tBin.data.data_ulong(seq);
			tBin.data.data_ulong().replace(bin->length(), bin->length(), reinterpret_cast<CORBA::ULong*>(data), false );	//no release
			success = true;
		}
	}
	else if (bin->isType<int*>()) {
		int* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_long_seq();
			tBin.data.data_long(seq);
			tBin.data.data_long().replace(bin->length(), bin->length(), reinterpret_cast<CORBA::Long*>(data), false );	//no release
			success = true;
		}
	}
	else if (bin->isType<float*>()) {
		float* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_float_seq();
			tBin.data.data_float(seq);
			tBin.data.data_float().replace(bin->length(), bin->length(), data, false );	//no release
			success = true;
		}
	}
	else if (bin->isType<double*>()) {
		double* data;
		if (bin->get(data)) {
			auto seq = STI::TNetwork::TMixedBinaryData::_data_double_seq();
			tBin.data.data_double(seq);
			tBin.data.data_double().replace(bin->length(), bin->length(), data, false );	//no release
			success = true;
		}
	}
	else if (bin->bytes() == 0) {
		auto seq = STI::TNetwork::TMixedBinaryData::_data_char_seq();
		tBin.data.data_char(seq);
		tBin.data.data_char().length(0);
		success = true;
	}
	return success;
}

template<>
bool STI::Network::convert<TBinaryData, std::shared_ptr<BinaryData>>(const TBinaryData& tBin, std::shared_ptr<BinaryData>& bin)
{
	return convertBinaryData(tBin, bin, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertBinaryData(
	const TBinaryData& tBin,
	std::shared_ptr<BinaryData>& bin,
	BinaryPayloadPolicy policy)
{
	if (bin == 0) return false;

	//TBinaryType { BinaryUChar, BinaryChar, BinaryUShort, BinaryShort, BinaryULong, BinaryLong, BinaryFloat, BinaryDouble };
	using STI::TNetwork::TBinaryType;

	bin->clear();
	bool release;
	size_t length;

	bool success = false;

	switch (tBin.data._d())
	{
	case TBinaryType::BinaryUChar:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_uchar();
			release = seq.release();
			length = seq.length();
			unsigned char* buffer = seq.get_buffer(release);	//orphan if release = true
			unsigned char* data = release ? buffer : copyBuffer(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryChar:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_char();
			release = seq.release();
			length = seq.length();
			unsigned char* buffer = seq.get_buffer(release);	//orphan if release = true
			char* dataChar = release ? reinterpret_cast<char*>(buffer) : copyBufferAs<char>(buffer, length);
			bin->assign(dataChar, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryUShort:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_ushort();
			release = seq.release();
			length = seq.length();
			unsigned short* buffer = seq.get_buffer(release);	//orphan if release = true
			unsigned short* data = release ? buffer : copyBuffer(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryShort:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_short();
			release = seq.release();
			length = seq.length();
			short* buffer = seq.get_buffer(release);	//orphan if release = true
			short* data = release ? buffer : copyBuffer(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryULong:
		{
			CORBA::ULong x;
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_ulong();
			release = seq.release();
			length = seq.length();
			CORBA::ULong* buffer = seq.get_buffer(release);	//orphan if release = true
			unsigned int* data = release ? reinterpret_cast<unsigned int*>(buffer) : copyBufferAs<unsigned int>(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryLong:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_long();
			release = seq.release();
			length = seq.length();
			CORBA::Long* buffer = seq.get_buffer(release);		//orphan if release = true
			int* data = release ? reinterpret_cast<int*>(buffer) : copyBufferAs<int>(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryFloat:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_float();
			release = seq.release();
			length = seq.length();
			float* buffer = seq.get_buffer(release);	//orphan if release = true
			float* data = release ? buffer : copyBuffer(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryDouble:
		{
			auto& seq = const_cast<TBinaryData&>(tBin).data.data_double();
			release = seq.release();
			length = seq.length();
			double* buffer = seq.get_buffer(release);	//orphan if release = true
			double* data = release ? buffer : copyBuffer(buffer, length);
			bin->assign(data, length);
			setBinaryMetadata(bin, tBin);
			success = true;
		}
		break;
	case TBinaryType::BinaryStream:
		{
			STI::TNetwork::TBinaryDataStream_var streamRef =
				STI::TNetwork::TBinaryDataStream::_duplicate(tBin.data.data_stream());
			std::shared_ptr<STI::Utils::BinaryDataStream> remoteDataStream 
				= std::make_shared<RemoteBinaryDataStream>(streamRef);

			if (policy == BinaryPayloadPolicy::PreserveStreamReference) {
				bin->attachStream(remoteDataStream, binaryLengthMetadata(tBin), binaryWordsizeMetadata(tBin));
				success = true;
			}
			else {
				std::shared_ptr<STI::Network::NetworkBinaryDataStreamTarget> networkTarget
					= std::make_shared<NetworkBinaryDataStreamTarget>(bin);

				remoteDataStream->transfer(networkTarget);
				setBinaryMetadata(bin, tBin);

				if (!bin->hasLocalData() && tBin.bytes == 0) {
					bin->allocate<char>(0);
					setBinaryMetadata(bin, tBin);
				}

				success = bin->hasLocalData() || tBin.bytes == 0;
			}
		}
		break;
	default:
		break;
	}

	return success;
}



template<>
bool STI::Network::convert<Image, TImage>(const Image& image, TImage& tImage)
{
	return convertImage(image, tImage, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertImage(const Image& image, TImage& tImage, BinaryPayloadPolicy policy)
{
	tImage.fileID = convert<FileID, TFileID>(image.getFileID());

	tImage.height = static_cast<CORBA::Long>(image.getHeight());
	tImage.width = static_cast<CORBA::Long>(image.getWidth());
	
	//convert meta data to list of string tuples
	auto metaData = image.metaData.getMetaData();
	tImage.metaData.length(metaData.getVector().size());

	unsigned i = 0;
	for (auto& tuple : metaData.getVector()) {
		auto key = tuple.getVector().at(0).getString();
		auto value = tuple.getVector().at(1).print();

		tImage.metaData[i].key = convert<std::string, ::CORBA::String_member>(key);
		tImage.metaData[i].value = convert<std::string, ::CORBA::String_member>(value);

		i++;
	}

	std::shared_ptr<STI::Utils::FileHolder> fileHolder;
	std::shared_ptr<BinaryData> bin;

	if (image.getFile(fileHolder)) {
		tImage.imageData.file(convert<FileID, TFileID>(fileHolder->getID()));
	}
	else if (image.getData(bin)) {
		TBinaryData tBin;
		convertBinaryData(bin, tBin, policy);	//no deep copy unless policy requires eager materialization
		tImage.imageData.binary(tBin);
	}
	else {
		//bin is null
		bin = std::make_shared<BinaryData>();
		TBinaryData tBin;
		convertBinaryData(bin, tBin, policy);

		tImage.imageData.binary(tBin);
	}

	return true;
}


template<>
bool STI::Network::convert<std::shared_ptr<Image>, TImage>(const std::shared_ptr<Image>& image, TImage& tImage)
{
	if (image == 0) return false;
	return convert<Image, TImage>(*image, tImage);
}

template<>
TImage STI::Network::convert<std::shared_ptr<Image>, TImage>(const std::shared_ptr<Image>& image)
{
	TImage tImage;
	convert<std::shared_ptr<Image>, TImage>(image, tImage);
	return tImage;
}


template<>
bool STI::Network::convert<TImage, Image>(const TImage& tImage, Image& image)
{
	return convertImage(tImage, image, BinaryPayloadPolicy::InlineOrEagerStream);
}

bool STI::Network::convertImage(const TImage& tImage, Image& image, BinaryPayloadPolicy policy)
{
	image.setFileID(convert<TFileID, FileID>(tImage.fileID));

	image.setHeight( static_cast<unsigned>(tImage.height) );
	image.setWidth( static_cast<unsigned>(tImage.width) );

	//meta data
	for (unsigned i = 0; i < tImage.metaData.length(); ++i) {
		auto key = convert<::CORBA::String_member, std::string>(tImage.metaData[i].key);
		auto value = convert<::CORBA::String_member, std::string>(tImage.metaData[i].value);

		image.metaData.addMetaData(key, value);
	}

	switch (tImage.imageData._d())
	{
		case TImageDataType::ImageDataBinary:
		{
			auto bin = std::make_shared<BinaryData>();
			image.setImageData(bin);

			if (bin != 0) {
				convertBinaryData(tImage.imageData.binary(), bin, policy);
			}
		}
		break;
		case TImageDataType::ImageDataFile:
		{
			FileID fileID;
			convert<TFileID, FileID>(tImage.imageData.file(), fileID);
			image.setFileID(fileID);
		}
		break;
	};

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
	case MixedValueType::Number:
		tType = TMixedValueType::MixedValueNumber;
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
	case TMixedValueType::MixedValueNumber:
		type = MixedValueType::Number;
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
