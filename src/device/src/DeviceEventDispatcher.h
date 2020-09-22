#ifndef STI_DEVICE_DEVICEEVENTDISPATCHER_H
#define STI_DEVICE_DEVICEEVENTDISPATCHER_H

#include "DeviceEvent.h"
#include "EventQueue.h"
#include "SynchronizedMap.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceEventHandler;

/// Devices push all locally generated events to the DeviceEventDispatcher where they are queued.  
/// The dispatcher holds references to remote event handlers that subscribe to specific event types.
class DeviceEventDispatcher : public STI::Utils::EventQueue<DeviceEvent>
{
public:

	DeviceEventDispatcher();
	~DeviceEventDispatcher();

	void clearAll();

	void addEventHandler(const DeviceID& id, const std::shared_ptr<DeviceEventHandler>& handler);
	void removeEventHandler(const DeviceID& id);

private:

	//EventQueue implementation
	void handleEvent(const DeviceEvent& evt);

	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventHandler>> handlers;
};

} //Device
} //STI


#endif

