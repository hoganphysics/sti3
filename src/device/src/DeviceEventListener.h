#ifndef STI_DEVICE_DEVICEEVENTLISTENER_H
#define STI_DEVICE_DEVICEEVENTLISTENER_H


namespace STI
{
namespace Device
{

template<class Event>
class DeviceEventListener
{
public:
	virtual void handleEvent(const Event& evt) = 0;
};


} //Device
} //STI


#endif
