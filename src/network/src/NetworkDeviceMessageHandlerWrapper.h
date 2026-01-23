#ifndef STI_NETWORK_NETWORKDEVICEMESSAGEHANDLERWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEMESSAGEHANDLERWRAPPER_H

#include "LocalDeviceMessageHandler.h"
#include "TDeviceMessageHandler_i.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageHandler.h>

#include "generated/orbTypes.h"
#include "ServantHolder.h"

#include <memory>


namespace STI
{
namespace Network
{


class NetworkDeviceMessageHandlerWrapper : public STI::Device::DeviceMessageHandler
{
public:

	NetworkDeviceMessageHandlerWrapper(const std::shared_ptr<STI::Device::LocalDeviceMessageHandler>& localHandler);
	~NetworkDeviceMessageHandlerWrapper();

	void addListenerGroup(const STI::Device::DeviceMessageType& type, std::shared_ptr<STI::Device::AbstractMessageListenerGroup>& listenerGroup) ;
	void removeListenerGroup(const STI::Device::DeviceMessageType& type);

	void addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess);
	void clearMessages();

	bool hasListeners(const std::shared_ptr<STI::Device::DeviceMessage>& mess);

	static bool getTDeviceMessageHandlerReference(
		const typename std::shared_ptr<STI::Device::DeviceMessageHandler>& messageHandler,
		STI::TNetwork::TDeviceMessageHandler_var& tMessageHandler)
	{
		auto wrapper = std::dynamic_pointer_cast<NetworkDeviceMessageHandlerWrapper>(messageHandler);
		if (wrapper) {
			tMessageHandler = wrapper->messageHandlerServantHolder.getRefVar();
			return !CORBA::is_nil(tMessageHandler);
		}
		return false;
	}

	void disable();

private:

	std::shared_ptr<STI::Device::LocalDeviceMessageHandler> localMessageHandler;

	// STI::TNetwork::TDeviceMessageHandler_i messageHandlerServant;
	ServantHolder<STI::TNetwork::TDeviceMessageHandler_i, STI::TNetwork::TDeviceMessageHandler> messageHandlerServantHolder;

};

} //Network
} //STI


#endif
