#ifndef STI_DEVICE_JDEVICEMESSAGEDISPATCHER_H
#define STI_DEVICE_JDEVICEMESSAGEDISPATCHER_H


#include <memory>


namespace STI
{
namespace Device
{

class DeviceMessage;
class DeviceMessageDispatcher;


//Java DeviceMessageDispatcher wrapper
class JDeviceMessageDispatcher
{
public:
	
	JDeviceMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher);
	~JDeviceMessageDispatcher();

	void addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess);
	void clearMessages();

private:

    std::shared_ptr<STI::Device::DeviceMessageDispatcher> localDispatcher;

};

} //Device
} //STI

#endif
