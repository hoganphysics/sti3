
#include "RemoteChannel.h"

#include "RemoteChannelManager.h"

using STI::Network::RemoteChannel;
using STI::Network::RemoteChannelManager;


RemoteChannel::RemoteChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, 
        const std::string& channelName, STI::Utils::MixedValue& metaData)
: channelNumber_(channelNumber), type_(type), inputType_(inputType), outputType_(outputType),
metaData_(metaData)
{
}

void RemoteChannel::attachManager(RemoteChannelManager* manager)
{
    remoteManager = manager;
}


short RemoteChannel::getChannelNumber() const
{
    return channelNumber_;
}

STI::Device::ChannelType RemoteChannel::getType() const
{
    return type_;
}

STI::Utils::MixedValueType RemoteChannel::getInputType() const
{
    return inputType_;
}

STI::Utils::MixedValueType RemoteChannel::getOutputType() const
{
    return outputType_;
}

void RemoteChannel::setChannelName(const std::string& name)
{
    if (remoteManager != 0) {
        remoteManager->setChannelName(channelNumber_, name);
    }
}

std::string RemoteChannel::getChannelName() const
{
    //return channelName_;
    
    std::string name = "";

    if (remoteManager != 0) {
        name = remoteManager->getChannelName(channelNumber_);
    }
    return name;
}

// void RemoteChannel::updateChannelName(const std::string& name)
// {
//     channelName_ = name;
// }

void RemoteChannel::saveLastValue(const STI::Utils::MixedValue& value)
{
    // lastValue = value;
}

const STI::Utils::MixedValue RemoteChannel::getLastValue() const
{
    STI::Utils::MixedValue lastValue;
    
    if (remoteManager != 0) {
        lastValue = remoteManager->getLastValue(channelNumber_);
    }

    return lastValue;
}

const STI::Utils::MixedValue& RemoteChannel::getMetaData() const
{
    return metaData_.getMetaData();
}

STI::Utils::MixedValue RemoteChannel::getMetaData(const std::string& key) const
{
    return metaData_.getMetaData(key);
}

