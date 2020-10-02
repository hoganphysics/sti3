#ifndef STI_DEVICE_JDEVICEEVENTDISPATCHER_H
#define STI_DEVICE_JDEVICEEVENTDISPATCHER_H


#include <memory>


namespace STI
{
namespace Device
{

class DeviceEvent;
class DeviceEventDispatcher;


//Java DeviceEventDispatcher wrapper
class JDeviceEventDispatcher
{
public:
	
	JDeviceEventDispatcher(const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher);
	~JDeviceEventDispatcher();

	void addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt);
	void clearEvents();

private:

    std::shared_ptr<STI::Device::DeviceEventDispatcher> localDispatcher;

};

} //Device
} //STI

#endif
