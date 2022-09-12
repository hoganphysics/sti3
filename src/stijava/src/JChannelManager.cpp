
#include "JChannelManager.h"
#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>
#include <sti/device/LocalChannel.h>

using STI::Device::JChannelManager;
using STI::Device::Channel;


JChannelManager::JChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager)
: localManager(manager)
{
}

JChannelManager::~JChannelManager()
{
}

std::vector<std::shared_ptr<Channel>> JChannelManager::getChannels()
{
    std::vector<std::shared_ptr<Channel>> channels;

    if (localManager != 0) {
        localManager->getChannels(channels);
    }
    return channels;
}

std::shared_ptr<Channel> JChannelManager::getChannel(short channelNumber)
{
    std::shared_ptr<STI::Device::Channel> channel;

    if (localManager != 0 && localManager->getChannel(channelNumber, channel)) {
        return channel;
    }

    //dummy
    channel = std::make_shared<STI::Device::LocalChannel>();
    return channel;
}

bool JChannelManager::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
    if (localManager != 0) {
        return localManager->writeChannel(channel, value);
    }
    return false;
}

STI::Utils::MixedValue JChannelManager::readChannel(short channel, const STI::Utils::MixedValue& value)
{
    STI::Utils::MixedValue data;

    if (localManager != 0) {
        return localManager->readChannel(channel, value, data);
    }
    return data;
}

