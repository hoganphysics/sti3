#ifndef STI_TNETWORK_TDEVICEEVENTDISPATCHER_I_H
#define STI_TNETWORK_TDEVICEEVENTDISPATCHER_I_H

#include "Device.h"
#include "DeviceEventDispatcher.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TDeviceEventDispatcher_i : public POA_STI::TNetwork::TDeviceEventDispatcher
{
public:

	TDeviceEventDispatcher_i(const std::shared_ptr<STI::Device::Device>& device);
	~TDeviceEventDispatcher_i();

	void addEventHandler(const ::STI::TNetwork::TDeviceID& targetID, ::STI::TNetwork::TDeviceEventHandler_ptr handler);
	void removeEventHandler(const ::STI::TNetwork::TDeviceID& targetID);


private:

	std::shared_ptr<STI::Device::DeviceEventDispatcher> eventDispatcher;
};

} //TNetwork
} //STI


#endif

