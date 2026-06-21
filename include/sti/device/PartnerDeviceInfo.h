#ifndef STI_DEVICE_PARTNERDEVICEINFO_H
#define STI_DEVICE_PARTNERDEVICEINFO_H

#include <sti/device/DeviceID.h>

#include <string>
#include <vector>

namespace STI
{
namespace Device
{

struct PartnerDeviceInfo
{
	DeviceID deviceID;
	std::vector<std::string> aliases;
	bool eventTarget = false;
};

} //Device
} //STI

#endif
