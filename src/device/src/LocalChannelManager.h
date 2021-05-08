#ifndef STI_DEVICE_LOCALCHANNELMANAGER_H
#define STI_DEVICE_LOCALCHANNELMANAGER_H

#include "ChannelManager.h"
#include "ChannelRefreshListener.h"
#include "SynchronizedMap.h"
#include "MessageGrouper.h"

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

    void addChannel(const std::shared_ptr<LocalChannel>& channel);


private:

    void handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value);
    void handleChannelNameRefreshEvent(short channelNumber, const std::string& name);

    LocalDevice* localDevice;

    STI::Utils::SynchronizedMap<short, std::shared_ptr<Channel>> channelMap;

    STI::Device::MessageGrouper<ChannelUpdateMessage> messageGrouper;
};


} //Device
} //STI

#endif

