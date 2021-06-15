#ifndef STI_DEVICE_GROUPINGDEVICEMESSAGEDISPATCHER_H
#define STI_DEVICE_GROUPINGDEVICEMESSAGEDISPATCHER_H

#include "DeviceMessageDispatcher.h"
#include "DeviceMessage.h"
#include "SynchronizedMap.h"

#include <memory>



namespace STI
{
namespace Device
{

class LocalDeviceMessageDispatcher;
class MessageGrouper;


class GroupingDeviceMessageDispatcher : public DeviceMessageDispatcher
{
public:

	GroupingDeviceMessageDispatcher();
	~GroupingDeviceMessageDispatcher();

	void addMessageHandler(const DeviceID& targetID, const std::shared_ptr<DeviceMessageHandler>& handler);
	void removeMessageHandler(const DeviceID& targetID);
	bool makeMessageHandler(std::shared_ptr<DeviceMessageHandler>& handler);

	void addMessage(const std::shared_ptr<DeviceMessage>& mess);
	void clearMessages();

    template<typename Message>
    void enableGrouping(STI::Device::DeviceMessageType messageType, int warmupTime_ms, int cooldownTime_ms)
    {
        MessageGrouper<Message> grouper(defaultDispatcher);
    //    messageGroupers[messageType] = 
    }

    void disableGrouping(STI::Device::DeviceMessageType messageType);

private:

    std::shared_ptr<LocalDeviceMessageDispatcher> defaultDispatcher;
    STI::Utils::SynchronizedMap<STI::Device::DeviceMessageType, STI::Device::MessageGrouper> messageGroupers;

};

} //Device
} //STI


#endif

