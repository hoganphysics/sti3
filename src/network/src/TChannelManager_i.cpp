
#include "TChannelManager_i.h"
#include "ORBManager.h"
#include "ChannelManager.h"

#include "Convert_Channel.h"
//#include "NetworkConvert.h"

using STI::Network::convert;
using STI::TNetwork::TChannelManager_i;
using STI::TNetwork::TChannelSeq;
using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;
using STI::Device::Channel;
using STI::TNetwork::TChannel;

TChannelManager_i::TChannelManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    device->getChannelManager(channelManager);
}

TChannelManager_i::~TChannelManager_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

TChannelSeq* TChannelManager_i::getChannels()
{
    STI::TNetwork::TChannelSeq_var tChannels( new TChannelSeq );

    std::vector<std::shared_ptr<STI::Device::Channel>> channels;

    if (channelManager != 0) {

		channelManager->getChannels(channels);

        convert<std::shared_ptr<STI::Device::Channel>, TChannel>(channels, 
                        (_CORBA_Unbounded_Sequence<STI::TNetwork::TChannel>&) tChannels);
	}

    return tChannels._retn();
}

::CORBA::Boolean TChannelManager_i::getChannel(::CORBA::Short channelNumber, ::STI::TNetwork::TChannel_out channel)
{
    bool success = false;

    std::shared_ptr<STI::Device::Channel> localChannel;

    if (channelManager != 0) {

		success = channelManager->getChannel(static_cast<short>(channelNumber), localChannel);
	}

    if (success) {
        channel = new STI::TNetwork::TChannel();
        
        success = convert<std::shared_ptr<STI::Device::Channel>, TChannel>(localChannel, (TChannel&)(*channel));
    }

    return success;
}

::CORBA::Boolean TChannelManager_i::writeChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value)
{
    bool success = false;

    if (channelManager != 0) {

		success = channelManager->writeChannel(static_cast<short>(channel), convert<TMixedValue, MixedValue>(value));
	}

    return success;
}

::CORBA::Boolean TChannelManager_i::readChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value, ::STI::TNetwork::TMixedValue_out data)
{
    bool success = false;
    MixedValue dataOut;

    if (channelManager != 0) {

		success = channelManager->readChannel(static_cast<short>(channel),
                                              convert<TMixedValue, MixedValue>(value), dataOut);
	}

    if (success) {
        data = new STI::TNetwork::TMixedValue();
        
        (*data) = convert<MixedValue, TMixedValue>(dataOut);
    }

    return success;
}

