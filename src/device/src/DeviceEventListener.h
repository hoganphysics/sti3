#ifndef STI_DEVICE_DEVICEEVENTLISTENER_H
#define STI_DEVICE_DEVICEEVENTLISTENER_H

#include <memory>

namespace STI
{
namespace Device
{

template<class Event>
class DeviceEventListener
{
public:
	virtual void handleEvent(const std::shared_ptr<Event>& evt) = 0;
};


} //Device
} //STI


#endif
