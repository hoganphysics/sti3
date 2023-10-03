#ifndef STI_NETWORK_NETWORKCONVERT_H
#define STI_NETWORK_NETWORKCONVERT_H

#include "ConvertList.h"
#include "generated/orbTypes.h"
#include <sti/fwd/MixedValue_fwd.h>
#include <sti/utils/GraphPathLabel.h>

#include <memory>
#include <type_traits>

namespace STI
{
namespace Network
{

//This template is only instantiated if a convert function is not specified.
//This will force a compiler error to indicated the convert is missing.
template<typename In, typename Out>
bool convertExists()
{
	return false;
}


template<typename In, typename Out>
bool convert(const In& input, Out& output)
{
	//Compiler error if an explicit specialization is not found.
	//Only specializations can be used.
	static_assert(convertExists<In, Out>(), "bool NetworkConvert::convert<In, Out> not specialized.");
	return false;
}


template<typename In, typename Out>
Out convert(const In& input)
{
	//Compiler error. This template should never be instantiated.
	//Only specializations can be used.
	static_assert(convertExists<In, Out>(), "Out NetworkConvert::convert<In, Out> not specialized.");
	return false;
}



//////////////// Lists //////////////////////


//_CORBA_Unbounded_Sequence_ObjRef< _objref_TFileHolder, _CORBA_ObjRef_Element< _objref_TFileHolder, TFileHolder_Helper> , TFileHolder_Helper >
//_CORBA_Unbounded_Sequence_ObjRef<TFileHolder_var::T_element::T_var::T_ptr*, TFileHolder_var::T_element, TFileHolder_Helper>

// template<typename In, typename Out, typename Out_Helper>
// bool convert(const std::vector<In>& input, typename _CORBA_Unbounded_Sequence_ObjRef<typename Out::T_element::T_var::T_ptr*, typename Out::T_element, Out_Helper>& output)
// {
// 	return false;
// }


template<typename In, typename Out>
bool convert(const std::vector<In>& input, _CORBA_Unbounded_Sequence<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence> convertlist;
	return convertlist(input, output);
}
template<typename In, typename Out>
bool convert(const std::vector<In>& input, _CORBA_Unbounded_Sequence_Forward<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence_Forward> convertlist;
	return convertlist(input, output);
}
template<typename In, typename Out>
bool convert(const std::set<In>& input, _CORBA_Unbounded_Sequence<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence> convertlist;
	return convertlist(input, output);
}
template<typename In, typename Out>
bool convert(const std::set<In>& input, _CORBA_Unbounded_Sequence_Forward<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence_Forward> convertlist;
	return convertlist(input, output);
}

template<typename In, typename Out, typename Any>
bool convertMap(const std::map<Any, In>& input, _CORBA_Unbounded_Sequence<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence> convertlist;
	return convertlist(input, output);
}

template<typename In, typename Out, typename Any>
bool convertMap(const std::map<Any, In>& input, _CORBA_Unbounded_Sequence_Forward<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence_Forward> convertlist;
	return convertlist(input, output);
}

template<typename In, typename Out>
bool convert(const _CORBA_Unbounded_Sequence<In>& input, std::set<Out>& output)
{
	for (unsigned i = 0; i < input.length(); ++i) {
		output.insert(convert<In, Out>(input[i]));
	}
	return (output.size() == input.length());
}

template<typename In, typename Out>
bool convert(const _CORBA_Unbounded_Sequence<In>& input, std::vector<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence> convertlist;
	return convertlist(input, output);
}

template<typename In, typename Out>
bool convert(const _CORBA_Unbounded_Sequence_Forward<In>& input, std::vector<Out>& output)
{
	ConvertList<In, Out, _CORBA_Unbounded_Sequence_Forward> convertlist;
	return convertlist(input, output);
}

bool convertBuffer(const char* buffer, unsigned length, ::STI::TNetwork::OctetSeq& tBuffer);
bool convertBuffer(const STI::TNetwork::OctetSeq& tBuffer, char* buffer);


bool convertEventGraphPath(const STI::Utils::GraphPathLabel& graphPath, ::STI::TNetwork::TGraphPathLabel& tGraphPath);
bool convertEventGraphPath(const ::STI::TNetwork::TGraphPathLabel& tGraphPath, STI::Utils::GraphPathLabel& graphPath);


} //Network



//////////////// Explicit Specializations //////////////////////

//Basic types

template<>
std::string Network::convert<::CORBA::String_member, std::string>(const ::CORBA::String_member& str);
template<>
::CORBA::String_member Network::convert<std::string, ::CORBA::String_member>(const std::string& str);

template<>
bool Network::convert<std::string, ::CORBA::String_member>(const std::string& str, ::CORBA::String_member& tString);


template<>
bool Network::convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(const std::vector<std::string>& stringVec, STI::TNetwork::TStringSeq& tStringSeq);
template<>
bool Network::convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(const STI::TNetwork::TStringSeq& tStringSeq, std::vector<std::string>& stringVec);







//can only list one way since the ::CORBA::UShort is just typedefed as unsigned short...
template<>
::CORBA::UShort Network::convert<unsigned short, ::CORBA::UShort>(const unsigned short& ushort);



template<>
bool Network::convert<std::map<std::string, std::string>, STI::TNetwork::TStringPairSeq>(
	const std::map<std::string, std::string>& stringMap, STI::TNetwork::TStringPairSeq& tStringMap);

template<>
bool Network::convert<STI::TNetwork::TStringPairSeq, std::map<std::string, std::string>>(
	const STI::TNetwork::TStringPairSeq& tStringMap, std::map<std::string, std::string>& stringMap);



namespace Device
{

class DeviceID;


} //Device

//DeviceID
template<> 
Device::DeviceID Network::convert<TNetwork::TDeviceID, Device::DeviceID>(const TNetwork::TDeviceID& tDeviceID);
template<>
TNetwork::TDeviceID Network::convert<Device::DeviceID, TNetwork::TDeviceID>(const Device::DeviceID& deviceID);
template<>
bool Network::convert<Device::DeviceID, TNetwork::TDeviceID>(const Device::DeviceID& deviceID, TNetwork::TDeviceID& tDeviceID);
template<>
bool Network::convert<TNetwork::TDeviceID, Device::DeviceID>(const TNetwork::TDeviceID& tDeviceID, Device::DeviceID& deviceID);


namespace Network
{

class HubID;
class HubTrace;


} //Network

//HubID
template<>
Network::HubID Network::convert<TNetwork::TDeviceHubID, Network::HubID>(const TNetwork::TDeviceHubID& tHubID);
template<>
TNetwork::TDeviceHubID Network::convert<Network::HubID, TNetwork::TDeviceHubID>(const Network::HubID& hubID);
template<>
bool Network::convert<Network::HubID, TNetwork::TDeviceHubID>(const Network::HubID& hubID, TNetwork::TDeviceHubID& tHubID);
template<>
bool Network::convert<TNetwork::TDeviceHubID, Network::HubID>(const TNetwork::TDeviceHubID& tHubID, Network::HubID& hubID);


//HubTrace
template<>
bool Network::convert<Network::HubTrace, TNetwork::TDeviceHubTrace>(const Network::HubTrace& hubTrace, TNetwork::TDeviceHubTrace& tHubTrace);
template<>
bool Network::convert<TNetwork::TDeviceHubTrace, Network::HubTrace>(const TNetwork::TDeviceHubTrace& tHubTrace, Network::HubTrace& hubTrace);
template<>
TNetwork::TDeviceHubTrace Network::convert<Network::HubTrace, TNetwork::TDeviceHubTrace>(const Network::HubTrace& hubTrace);
template<>
Network::HubTrace Network::convert<TNetwork::TDeviceHubTrace, Network::HubTrace>(const TNetwork::TDeviceHubTrace& tDeviceHubTrace);



namespace Utils
{

class MixedValue;
class BinaryData;
class Image;

} //Utils


//MixedValue
template<>
TNetwork::TMixedValue Network::convert<Utils::MixedValue, TNetwork::TMixedValue>(const Utils::MixedValue& value);
template<>
Utils::MixedValue Network::convert<TNetwork::TMixedValue, Utils::MixedValue>(const TNetwork::TMixedValue& tValue);

template<>
bool Network::convert<Utils::MixedValue, TNetwork::TMixedValue>(const Utils::MixedValue& value, TNetwork::TMixedValue& tValue);
template<>
bool Network::convert<TNetwork::TMixedValue, Utils::MixedValue>(const TNetwork::TMixedValue& tValue, Utils::MixedValue& value);

template<>
TNetwork::TMixedValueType Network::convert<Utils::MixedValueType, TNetwork::TMixedValueType>(const Utils::MixedValueType& type);
template<>
Utils::MixedValueType Network::convert<TNetwork::TMixedValueType, Utils::MixedValueType>(const TNetwork::TMixedValueType& tType);


template<>
bool Network::convert<std::shared_ptr<Utils::BinaryData>, TNetwork::TBinaryData>(const std::shared_ptr<Utils::BinaryData>& bin, TNetwork::TBinaryData& tBin);
template<>
bool Network::convert<TNetwork::TBinaryData, std::shared_ptr<Utils::BinaryData>>(const TNetwork::TBinaryData& tBin, std::shared_ptr<Utils::BinaryData>& bin);

template<>
bool Network::convert<Utils::Image, TNetwork::TImage>(const Utils::Image& image, TNetwork::TImage& tImage);
template<>
bool Network::convert<TNetwork::TImage, Utils::Image>(const TNetwork::TImage& tImage, Utils::Image& image);

template<>
bool Network::convert<std::shared_ptr<Utils::Image>, TNetwork::TImage>(const std::shared_ptr<Utils::Image>& image, TNetwork::TImage& tImage);
template<>
TNetwork::TImage Network::convert<std::shared_ptr<Utils::Image>, TNetwork::TImage>(const std::shared_ptr<Utils::Image>& image);



} //STI

#endif

