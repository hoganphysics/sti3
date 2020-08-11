
#include "NetworkConvert.h"

#include "DeviceID.h"

#include "orbTypes.h"

using STI::Network::convert;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;

template<>
DeviceID STI::Network::convert<TDeviceID, DeviceID>(const TDeviceID& tDeviceID)
{
	using std::string;

	return DeviceID(
		convert<CORBA::String_member, std::string>(tDeviceID.deviceName),
		convert<CORBA::String_member, std::string>(tDeviceID.address),
		static_cast<unsigned short>(tDeviceID.moduleNum),
		convert<CORBA::String_member, std::string>(tDeviceID.targetServer)
	);
}

template<>
TDeviceID STI::Network::convert<DeviceID, TDeviceID>(const DeviceID& deviceID)
{
	TDeviceID tDeviceID;

	convert<DeviceID, TDeviceID>(deviceID, tDeviceID);

	return tDeviceID;

}

template<>
bool STI::Network::convert<DeviceID, TDeviceID>(const DeviceID& deviceID, TDeviceID& tDeviceID)
{
	using std::string;
	using ::CORBA::String_member;

	tDeviceID.address = convert<string, String_member>(deviceID.getAddress());
	tDeviceID.deviceName = convert<string, String_member>(deviceID.getName());
	tDeviceID.moduleNum = convert<unsigned short, ::CORBA::UShort>(deviceID.getModule());

	return true;
}

