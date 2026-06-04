#include "Convert_Channel.h"
#include "RemoteChannel.h"

#include <sti/utils/MixedValue.h>

#include "generated/orbTypes.h"

using STI::Network::convert;
using STI::Device::Channel; 
using STI::TNetwork::TChannel;
using STI::Utils::MixedValueType;
using STI::TNetwork::TMixedValueType;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::Device::ChannelType;
using STI::TNetwork::TChannelType;
using STI::Network::RemoteChannel;


//Channel
template<>
bool STI::Network::convert<std::shared_ptr<Channel>, TChannel>(const std::shared_ptr<Channel>& channel, TChannel& tChannel)
{
    bool success = false;

    if (channel != 0) {
        
        tChannel.channelName = convert<std::string, ::CORBA::String_member>(channel->getChannelName());
        tChannel.channelNumber = static_cast<CORBA::Short>(channel->getChannelNumber());
        tChannel.inputType = convert<MixedValueType, TMixedValueType>(channel->getInputType());
        tChannel.outputType = convert<MixedValueType, TMixedValueType>(channel->getOutputType());
        tChannel.type = convert<ChannelType, TChannelType>(channel->getType());
        tChannel.metaData = convert<MixedValue, TMixedValue>(channel->getMetaData());
        tChannel.lastValue = convert<MixedValue, TMixedValue>(channel->getLastValue());
        STI::Network::convertMixedValue(channel->getLastMeasurement(), tChannel.lastMeasurement,
            STI::Network::BinaryPayloadPolicy::PreferStreamReference);

        success = true;
    }

    return success;
}

template<>
bool STI::Network::convert<TChannel, std::shared_ptr<RemoteChannel>>(const TChannel& tChannel, std::shared_ptr<RemoteChannel>& channel)
{
    channel = STI::Network::convert<TChannel, std::shared_ptr<RemoteChannel>>(tChannel);

    return (channel != 0);
}

template<>
std::shared_ptr<RemoteChannel> STI::Network::convert<TChannel, std::shared_ptr<RemoteChannel>>(const TChannel& tChannel)
{

    //Meta data
    MixedValue metaData = convert<TMixedValue, MixedValue>(tChannel.metaData);
    // const STI::Utils::MixedValueVector& metaValues = metaData.getVector();

    MixedValue lastValue = convert<TMixedValue, MixedValue>(tChannel.lastValue);
    MixedValue lastMeasurement;
    STI::Network::convertMixedValue(tChannel.lastMeasurement, lastMeasurement,
        STI::Network::BinaryPayloadPolicy::PreserveStreamReference);

    auto remoteChannel = std::make_shared<STI::Network::RemoteChannel>(
                            static_cast<short>(tChannel.channelNumber),
                            convert<TChannelType, ChannelType>(tChannel.type),
                            convert<TMixedValueType, MixedValueType>(tChannel.inputType),
                            convert<TMixedValueType, MixedValueType>(tChannel.outputType),
                            convert<::CORBA::String_member, std::string>(tChannel.channelName),
                            lastValue, lastMeasurement, metaData);



    // for (auto& tuple : metaValues) {
        
    //     const STI::Utils::MixedValueVector& labeledData = tuple.getVector();
        
    //     if (labeledData.size() == 2) {
    //         localChannel->addMetaData(labeledData.at(0).getString(), labeledData.at(1));
    //     }
    // }

    return remoteChannel;
}



//ChannelType
template<>
TChannelType STI::Network::convert<ChannelType, TChannelType>(const ChannelType& type)
{
    TChannelType tType;

    switch (type)
    {
    case ChannelType::Output:
        tType = TChannelType::TChannelOutput;
        break;
    case ChannelType::Input:
        tType = TChannelType::TChannelInput;
        break;
    default:
        tType = TChannelType::TChannelOutput;
        break;
    }

    return tType;
}


template<>
ChannelType STI::Network::convert<TChannelType, ChannelType>(const TChannelType& tType)
{
    ChannelType type;

    switch (tType)
    {
    case TChannelType::TChannelOutput:
        type = ChannelType::Output;
        break;
    case TChannelType::TChannelInput:
        type = ChannelType::Input;
        break;
    default:
        type = ChannelType::Output;
        break;
    }

    return type;
}
