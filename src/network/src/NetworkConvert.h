#ifndef STI_NETWORK_NETWORKCONVERT_H
#define STI_NETWORK_NETWORKCONVERT_H

#include "ConvertList.h"
#include "orbTypes.h"

#include <memory>

namespace STI
{
namespace Network
{

template<typename In, typename Out>
bool convert(const In& input, Out& output)
{
	//Compiler error if an explicit specialization is not found.
}

template<typename In, typename Out>
Out convert(const In& input)
{
	//Compiler error. This template should never be instantiated.
	//Only specializations can be used.
}


//////////////// Lists //////////////////////

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


} //Network



//////////////// Explicit Specializations //////////////////////

//Basic types

template<>
std::string Network::convert<::CORBA::String_member, std::string>(const ::CORBA::String_member& str);
template<>
::CORBA::String_member Network::convert<std::string, ::CORBA::String_member>(const std::string& str);

//can only list one way since the ::CORBA::UShort is just typedefed as unsigned short...
template<>
::CORBA::UShort Network::convert<unsigned short, ::CORBA::UShort>(const unsigned short& ushort);



namespace Device
{

class DeviceID;
class DeviceEvent;
class RefreshDeviceEvent;
enum class DeviceEventType;

} //Device

//DeviceID
template<> 
Device::DeviceID Network::convert<TNetwork::TDeviceID, Device::DeviceID>(const TNetwork::TDeviceID& tDeviceID);
template<>
TNetwork::TDeviceID Network::convert<Device::DeviceID, TNetwork::TDeviceID>(const Device::DeviceID& deviceID);
template<>
bool Network::convert<Device::DeviceID, TNetwork::TDeviceID>(const Device::DeviceID& deviceID, TNetwork::TDeviceID& tDeviceID);


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

//HubTrace
template<>
bool Network::convert<Network::HubTrace, TNetwork::TDeviceHubTrace>(const Network::HubTrace& hubTrace, TNetwork::TDeviceHubTrace& tHubTrace);
template<>
bool Network::convert<TNetwork::TDeviceHubTrace, Network::HubTrace>(const TNetwork::TDeviceHubTrace& tHubTrace, Network::HubTrace& hubTrace);
template<>
TNetwork::TDeviceHubTrace Network::convert<Network::HubTrace, TNetwork::TDeviceHubTrace>(const Network::HubTrace& hubTrace);
template<>
Network::HubTrace Network::convert<TNetwork::TDeviceHubTrace, Network::HubTrace>(const TNetwork::TDeviceHubTrace& tDeviceHubTrace);

//DeviceEvent

template<>
bool Network::convert<std::shared_ptr<STI::Device::DeviceEvent>, TNetwork::TDeviceEvent>(const std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent, TNetwork::TDeviceEvent& tEvent);
template<>
bool Network::convert<TNetwork::TDeviceEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TNetwork::TDeviceEvent& tEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent);

template<>
bool Network::convert<std::shared_ptr<STI::Device::DeviceEvent>, TNetwork::TAnyEvent>(const std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent, TNetwork::TAnyEvent& tAnyEvent);
template<>
bool Network::convert<TNetwork::TAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TNetwork::TAnyEvent& tAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent);

template<>
TNetwork::TDeviceEventType Network::convert<Device::DeviceEventType, TNetwork::TDeviceEventType>(const Device::DeviceEventType& type);
template<>
Device::DeviceEventType Network::convert<TNetwork::TDeviceEventType, Device::DeviceEventType>(const TNetwork::TDeviceEventType& tType);
template<>
bool Network::convert<Device::DeviceEventType, TNetwork::TDeviceEventType>(const Device::DeviceEventType& type, TNetwork::TDeviceEventType& tType);
template<>
bool Network::convert<TNetwork::TDeviceEventType, Device::DeviceEventType>(const TNetwork::TDeviceEventType& tType, Device::DeviceEventType& type);


template<>
bool Network::convert<TNetwork::TRefreshDeviceEvent, std::shared_ptr<Device::RefreshDeviceEvent>>(const TNetwork::TRefreshDeviceEvent& tEvent, std::shared_ptr<Device::RefreshDeviceEvent>& deviceEvent);


} //STI

#endif

