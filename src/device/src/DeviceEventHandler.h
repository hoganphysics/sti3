#ifndef STI_DEVICE_DEVICEEVENTHANDLER_H
#define STI_DEVICE_DEVICEEVENTHANDLER_H

#include "DeviceEvent.h"
#include "DeviceEventListener.h"
#include "SynchronizedMap.h"
#include "EventQueue.h"
#include "DeviceEventListenerGroup.h"

#include <set>
#include <typeinfo>
#include <vector>
#include <memory>

namespace STI
{
namespace Device
{

class DeviceEventHandler
{
public:

	virtual ~DeviceEventHandler() {}

	virtual void addListenerGroup(const DeviceEventType& type, std::shared_ptr<AbstractEventListenerGroup>& listenerGroup) = 0;
	virtual void removeListenerGroup(const DeviceEventType& type) = 0;

	virtual void addEvent(const std::shared_ptr<DeviceEvent>& evt) = 0;
	virtual void clearEvents() = 0;

	virtual bool hasListeners(const std::shared_ptr<DeviceEvent>& evt) = 0;

};


} //Device
} //STI


#endif

