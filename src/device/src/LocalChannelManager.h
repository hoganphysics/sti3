#ifndef STI_DEVICE_LOCALCHANNELMANAGER_H
#define STI_DEVICE_LOCALCHANNELMANAGER_H

#include <sti/device/ChannelManager.h>
#include <sti/utils/SynchronizedMap.h>

#include "ChannelRefreshListener.h"
#include "DeviceMessageGrouper.h"

#include <memory>


namespace STI
{
namespace Device
{

class LocalDevice;
class DeviceMessageDispatcher;
class ChannelUpdateMessage;
class LocalChannel;


class LocalChannelManager : public ChannelManager,
                            public ChannelRefreshListener
{
public:

	LocalChannelManager(LocalDevice* localDevice, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	~LocalChannelManager() {}

    void getChannels(std::vector<std::shared_ptr<Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
    void stop();

    void addChannel(const std::shared_ptr<LocalChannel>& channel);


private:

    void handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value);
    void handleChannelNameRefreshEvent(short channelNumber, const std::string& name);

    LocalDevice* localDevice;

    STI::Utils::SynchronizedMap<short, std::shared_ptr<Channel>> channelMap;

    STI::Device::DeviceMessageGrouper<ChannelUpdateMessage> messageGrouper;
};


} //Device
} //STI

#endif

