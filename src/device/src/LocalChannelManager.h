#ifndef STI_DEVICE_LOCALCHANNELMANAGER_H
#define STI_DEVICE_LOCALCHANNELMANAGER_H

#include "ChannelManager.h"
#include "SynchronizedMap.h"

#include <memory>

namespace STI
{
namespace Device
{

class LocalDevice;


class LocalChannelManager : public ChannelManager
{
public:

	LocalChannelManager(LocalDevice* localDevice);
	~LocalChannelManager() {}

    void getChannels(std::vector<std::shared_ptr<Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

    void addChannel(const std::shared_ptr<Channel>& channel);


private:

    LocalDevice* localDevice;

    STI::Utils::SynchronizedMap<short, std::shared_ptr<Channel>> channelMap;

};


} //Device
} //STI

#endif

