#include "LocalChannelManager.h"

#include <sti/LocalDevice.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalChannel.h>

using STI::Device::Channel;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::LocalChannel;
using STI::Device::LocalChannelManager;
using STI::Device::LocalDevice;
using STI::Utils::MixedValue;


LocalChannelManager::LocalChannelManager(LocalDevice* localDevice, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
 : localDevice(localDevice), messageGrouper(dispatcher)
{
    messageGrouper.setWarmup(100);   //ms
    messageGrouper.setCooldown(500); //ms

    messageGrouper.start();
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

void LocalChannelManager::stop()
{
    localDevice->stopRW();
}

void LocalChannelManager::addChannel(const std::shared_ptr<LocalChannel>& channel)
{
    if (channel != 0) {
        channelMap.add(channel->getChannelNumber(), channel);
        channel->addRefreshListener(this);
    }
}

void LocalChannelManager::handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value)
{
    auto message = std::make_shared<STI::Device::ChannelUpdateMessage>(localDevice->getID(), channelNumber, value);
    messageGrouper.addMessage(message);
}

void LocalChannelManager::handleChannelNameRefreshEvent(short channelNumber, const std::string& name)
{
    auto message = std::make_shared<STI::Device::ChannelUpdateMessage>(localDevice->getID(), channelNumber, name);
    messageGrouper.addMessage(message);
}

