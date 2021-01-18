#ifndef STI_NETWORK_REMOTEDEVICEMESSAGEHANDLER_H
#define STI_NETWORK_REMOTEDEVICEMESSAGEHANDLER_H

#include "deviceNet.h"
#include "DeviceMessageHandler.h"
#include "TRefreshIndicator_i.h"

#include <memory>
#include <set>
#include <mutex>

namespace STI
{
namespace Network
{

class RemoteDeviceMessageHandler : public STI::Device::DeviceMessageHandler
{
public:
	
	RemoteDeviceMessageHandler(::STI::TNetwork::TDeviceMessageHandler_ptr deviceHandler);
	~RemoteDeviceMessageHandler();

	void addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess);
	void clearMessages();

	bool hasListeners(const std::shared_ptr<STI::Device::DeviceMessage>& mess);

private:

	void addListenerGroup(const STI::Device::DeviceMessageType& type, std::shared_ptr<STI::Device::AbstractMessageListenerGroup>& listenerGroup);
	void removeListenerGroup(const STI::Device::DeviceMessageType& type);

	::STI::TNetwork::TDeviceMessageHandler_var tDeviceHandler;		//remote reference

	::STI::TNetwork::TRefreshIndicator_i refreshIndicator;		//records if the remote resource refreshed

	std::set<STI::Device::DeviceMessageType> listenersTypes;		//set of all event types this handler responds to

	mutable std::mutex listenersMutex;

};


} //Network
} //STI


#endif



