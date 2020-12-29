

#include "LocalChannelManager.h"
#include "Channel.h"
#include "LocalDevice.h"

using STI::Device::LocalChannelManager;
using STI::Device::LocalDevice;
using STI::Device::Channel;
using STI::Utils::MixedValue;

LocalChannelManager::LocalChannelManager(LocalDevice* localDevice)
 : localDevice(localDevice)
{
}


void LocalChannelManager::getChannels(std::vector<std::shared_ptr<Channel>>& channels)
{
    channels.clear();
    std::set<short> keys;

    channelMap.getKeys(keys);

    std::shared_ptr<Channel> channel;

    for (auto& key : keys) {
        if (channelMap.get(key, channel)) {
            channels.push_back(channel);
        }
    }
}

bool LocalChannelManager::getChannel(short channelNumber, std::shared_ptr<Channel>& channel)
{
    return (channelMap.get(channelNumber, channel) && channel != 0);
}


bool LocalChannelManager::writeChannel(short channel, const MixedValue& value)
{
    std::shared_ptr<Channel> ch;

    if (channelMap.get(channel, ch) && ch != 0 && localDevice->write(channel, value)) {
        ch->saveLastValue(value);
        return true;
    }
    return false;
}

bool LocalChannelManager::readChannel(short channel, const MixedValue& value, MixedValue& data)
{
    std::shared_ptr<Channel> ch;

    if (channelMap.get(channel, ch) && ch != 0 && localDevice->read(channel, value, data)) {
        ch->saveLastValue(data);
        return true;
    }
    return false;
}

void LocalChannelManager::addChannel(const std::shared_ptr<Channel>& channel)
{
    if (channel != 0) {
        channelMap.add(channel->getChannelNumber(), channel);
    }
}

