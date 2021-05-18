#ifndef STI_TNETWORK_TDEVICEMESSAGEDISPATCHER_I_H
#define STI_TNETWORK_TDEVICEMESSAGEDISPATCHER_I_H

#include "Device.h"
#include "DeviceMessageDispatcher.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TDeviceMessageDispatcher_i : public POA_STI::TNetwork::TDeviceMessageDispatcher
{
public:

	TDeviceMessageDispatcher_i(const std::shared_ptr<STI::Device::Device>& device);
	~TDeviceMessageDispatcher_i();

	void addMessageHandler(const ::STI::TNetwork::TDeviceID& targetID, ::STI::TNetwork::TDeviceMessageHandler_ptr handler);
	void removeMessageHandler(const ::STI::TNetwork::TDeviceID& targetID);

	::CORBA::Boolean ping();
	
private:

	std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;
};

} //TNetwork
} //STI


#endif

