#include "LocalChannelManager.h"

#include <sti/LocalDevice.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalChannel.h>

#include <sti/utils/Configuration.h>
#include <sti/utils/TimeStamp.h>

#include <sstream>

using STI::Device::Channel;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::LocalChannel;
using STI::Device::LocalChannelManager;
using STI::Device::LocalDevice;
using STI::Utils::MixedValue;
using STI::Utils::Configuration;


LocalChannelManager::LocalChannelManager(LocalDevice* localDevice, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
 : localDevice(localDevice), messageGrouper(dispatcher), loading(false)
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
    if (channel != 0 && channelMap.add(channel->getChannelNumber(), channel)) {
        channel->addRefreshListener(this);

        //add any new keys to persistence
        std::string ch = STI::Utils::valueToString(channel->getChannelNumber());
        if (persistenceData != 0 && !persistenceData->includes("Channel names", ch)) {           
            persistenceData->set("Channel names", ch, channel->getChannelName());
        }
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

    //persistence
    if (persistenceData != 0 && !loading.load()) {
        std::string ch = STI::Utils::valueToString(channelNumber);
        persistenceData->set("Channel names", ch, name);

        if (persistenceRefresher) {
            persistenceRefresher();
        }
    }
}


bool LocalChannelManager::loadProfile(const std::shared_ptr<Profile>& profile)
{
    if (profile == 0) return false;

    if (profile->type != ProfileType::All && profile->type != ProfileType::Channel) return true;

    bool success = true;

    for (auto& channel : profile->channelData) {
        success &= writeChannel(channel.first, channel.second);
    }

    return success;
}

bool LocalChannelManager::saveProfile(const std::shared_ptr<Profile>& profile)
{
    if (profile == 0) return false;

    if (profile->type != ProfileType::All && profile->type != ProfileType::Channel) return true;

    std::vector<std::shared_ptr<Channel>> channels;
    getChannels(channels);

    profile->channelData.clear();

    for (auto& ch : channels) {
        if (ch->getType() == ChannelType::Output) {
            profile->channelData[ch->getChannelNumber()] = ch->getLastValue();
        }
    }

    return true;
}


//*********** PersistenceTarget ****************//

std::string LocalChannelManager::getFilenameStem()
{
    return "channels";
}

std::string LocalChannelManager::getHeader()
{
    auto localID = localDevice->getID();

    std::stringstream header;
    header << "Channel information for " << localID.getID() << std::endl;
    header << "  Name: " << localID.getName() << std::endl;
    header << "  Address: " << localID.getAddress() << std::endl;
    header << "  Module: " << localID.getModule() << std::endl;

    STI::Utils::TimeStamp timestamp;
    header << "Last saved: " << timestamp.print() << std::endl;

    return header.str();
}

void LocalChannelManager::setPersistenceCallback(const std::function<void(void)>& refresher)
{
    persistenceRefresher = refresher;
}

void LocalChannelManager::setPersistenceData(const std::shared_ptr<STI::Utils::Configuration>& data)
{
    persistenceData = data;
}

bool LocalChannelManager::save()
{
    return true;    //persistenceData is kept current with each refresh event
}

void LocalChannelManager::load()
{
    loading = true;

    if (persistenceData == 0) return;

    auto storedKeys = persistenceData->getParameterNames("Channel names");

    for (auto& key : storedKeys) {
        std::string value;
        if (persistenceData->getParameter("Channel names", key, value)) {
            short channelNumber;
            std::shared_ptr<Channel> channel;

            if (STI::Utils::stringToValue(key, channelNumber) && getChannel(channelNumber, channel)) {
                channel->setChannelName(value);
            }
        }
    }
    loading = false;
}
