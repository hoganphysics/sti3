#ifndef STI_NETWORK_NETWORKDEVICEEVENTHANDLERWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEEVENTHANDLERWRAPPER_H

#include "DeviceEventHandler.h"
#include "LocalDeviceEventHandler.h"
#include "TDeviceEventHandler_i.h"
#include "DeviceEvent.h"

#include "orbTypes.h"

#include <memory>

namespace STI
{
namespace Network
{

//Maybe NetworkMessageHandlerWrapper, RemoteMessageHandler, MessageDispatcher, MessageReceiver, etc.

//thin wrapper around LocalDeviceEventHandler that also holds the TDeviceEventHandler_i servant of the same Handler
class NetworkDeviceEventHandlerWrapper : public STI::Device::DeviceEventHandler
{
public:

	NetworkDeviceEventHandlerWrapper(const std::shared_ptr<STI::Device::LocalDeviceEventHandler>& localHandler);
	~NetworkDeviceEventHandlerWrapper();

	void addListenerGroup(const STI::Device::DeviceEventType& type, std::shared_ptr<STI::Device::AbstractEventListenerGroup>& listenerGroup) ;
	void removeListenerGroup(const STI::Device::DeviceEventType& type);

	void addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt);
	void clearEvents();

	bool hasListeners(const std::shared_ptr<STI::Device::DeviceEvent>& evt);

	static bool getTDeviceEventHandlerReference(
		const typename std::shared_ptr<STI::Device::DeviceEventHandler>& eventHandler,
		STI::TNetwork::TDeviceEventHandler_ptr& tEventHandler)
	{
		auto wrapper = std::dynamic_pointer_cast<NetworkDeviceEventHandlerWrapper>(eventHandler);
		if (wrapper) {
			tEventHandler = wrapper->eventHandlerServant._this();
			return !CORBA::is_nil(tEventHandler);
		}
		return false;
	}

private:

	std::shared_ptr<STI::Device::LocalDeviceEventHandler> localEventHandler;

	STI::TNetwork::TDeviceEventHandler_i eventHandlerServant;

};

} //Network
} //STI


#endif

