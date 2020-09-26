#ifndef STI_DEVICE_DEVICEEVENTDISPATCHER_H
#define STI_DEVICE_DEVICEEVENTDISPATCHER_H


#include <memory>

namespace STI
{
namespace Device
{

class DeviceID;
class DeviceEvent;
class DeviceEventHandler;


class DeviceEventDispatcher
{
public:
	
	virtual ~DeviceEventDispatcher() {}

	virtual void addEventHandler(const DeviceID& targetID, const std::shared_ptr<DeviceEventHandler>& handler) = 0;
	virtual void removeEventHandler(const DeviceID& targetID) = 0;
	virtual bool makeEventHandler(std::shared_ptr<DeviceEventHandler>& handler) = 0;

	virtual void addEvent(const std::shared_ptr<DeviceEvent>& evt) = 0;
	virtual void clearEvents() = 0;

};


} //Device
} //STI


#endif

