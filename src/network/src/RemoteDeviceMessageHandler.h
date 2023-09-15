#ifndef STI_NETWORK_REMOTEDEVICEMESSAGEHANDLER_H
#define STI_NETWORK_REMOTEDEVICEMESSAGEHANDLER_H

#include "generated/deviceNet.h"
#include <sti/device/DeviceMessageHandler.h>
#include "TRefreshIndicator_i.h"
#include "TReferenceHolder.h"

#include <memory>
#include <set>
#include <mutex>


namespace STI
{
namespace Network
{

class RemoteDeviceMessageHandler : public STI::Device::DeviceMessageHandler,
								   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceMessageHandler>	//mixin
{
public:
	
	RemoteDeviceMessageHandler(::STI::TNetwork::TDeviceMessageHandler_ptr deviceHandler);
	~RemoteDeviceMessageHandler();

	void addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess);
	void clearMessages();

	bool hasListeners(const std::shared_ptr<STI::Device::DeviceMessage>& mess);

	void disable();

private:

	void addListenerGroup(const STI::Device::DeviceMessageType& type, std::shared_ptr<STI::Device::AbstractMessageListenerGroup>& listenerGroup);
	void removeListenerGroup(const STI::Device::DeviceMessageType& type);

	::STI::TNetwork::TRefreshIndicator_i refreshIndicator;		//records if the remote resource refreshed
	std::set<STI::Device::DeviceMessageType> listenersTypes;		//set of all event types this handler responds to

	mutable std::mutex handlerMutex;
};


} //Network
} //STI


#endif



