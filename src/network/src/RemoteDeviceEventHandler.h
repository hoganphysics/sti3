#ifndef STI_NETWORK_REMOTEDEVICEEVENTHANDLER_H
#define STI_NETWORK_REMOTEDEVICEEVENTHANDLER_H

#include "deviceNet.h"
#include "DeviceEventHandler.h"
#include "TRefreshIndicator_i.h"

#include <memory>
#include <set>
#include <mutex>

namespace STI
{
namespace Network
{

class RemoteDeviceEventHandler : public STI::Device::DeviceEventHandler
{
public:
	
	RemoteDeviceEventHandler(::STI::TNetwork::TDeviceEventHandler_ptr deviceHandler);
	~RemoteDeviceEventHandler();

	void addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt);
	void clearEvents();

	bool hasListeners(const std::shared_ptr<STI::Device::DeviceEvent>& evt);

private:

	void addListenerGroup(const STI::Device::DeviceEventType& type, std::shared_ptr<STI::Device::AbstractEventListenerGroup>& listenerGroup);
	void removeListenerGroup(const STI::Device::DeviceEventType& type);

	::STI::TNetwork::TDeviceEventHandler_var tDeviceHandler;		//remote reference

	::STI::TNetwork::TRefreshIndicator_i refreshIndicator;		//records if the remote resource refreshed

	std::set<STI::Device::DeviceEventType> listenersTypes;		//set of all event types this handler responds to

	mutable std::mutex listenersMutex;

};


} //Network
} //STI


#endif



