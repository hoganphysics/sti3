
#include "TChannelManager_i.h"
#include "ORBManager.h"
#include <sti/device/ChannelManager.h>

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
    if (device != 0) {
        device->getChannelManager(channelManager);        
    }
}

TChannelManager_i::~TChannelManager_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

// TChannelSeq* TChannelManager_i::getChannels()
void TChannelManager_i::getChannels(::STI::TNetwork::TChannelSeq_out channels)
{
    STI::TNetwork::TChannelSeq_var tChannelSeq_var( new TChannelSeq );
    std::vector<std::shared_ptr<STI::Device::Channel>> localChannels;
    channels = new STI::TNetwork::TChannelSeq();

    if (channelManager != 0) {

		channelManager->getChannels(localChannels);

        convert<std::shared_ptr<STI::Device::Channel>, TChannel>(localChannels, tChannelSeq_var);

		(*channels) = tChannelSeq_var;
	}
}

::CORBA::Boolean TChannelManager_i::getChannel(::CORBA::Short channelNumber, ::STI::TNetwork::TChannel_out channel)
{
    bool success = false;

    std::shared_ptr<STI::Device::Channel> localChannel;
    channel = new STI::TNetwork::TChannel();

    if (channelManager != 0) {

		success = channelManager->getChannel(static_cast<short>(channelNumber), localChannel);
	}

    if (success) {
       
        success = convert<std::shared_ptr<STI::Device::Channel>, TChannel>(localChannel, (TChannel&)(*channel));
    }

    return success;
}

::CORBA::Boolean TChannelManager_i::setChannelName(::CORBA::Short channelNumber, const char* name)
{
    bool success = false;

    std::shared_ptr<STI::Device::Channel> localChannel;

    if (channelManager != 0 
        && channelManager->getChannel(static_cast<short>(channelNumber), localChannel)
        && localChannel != 0)
    {
        localChannel->setChannelName(convert<CORBA::String_member, std::string>(name));
        success = true;
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
    data = new STI::TNetwork::TMixedValue();

    if (channelManager != 0) {

		success = channelManager->readChannel(static_cast<short>(channel),
                                              convert<TMixedValue, MixedValue>(value), dataOut);
	}

    if (success) {
        
        (*data) = convert<MixedValue, TMixedValue>(dataOut);
    }

    return success;
}

void TChannelManager_i::stop()
{
    if (channelManager != 0) {
		channelManager->stop();
	}
}

::CORBA::Boolean TChannelManager_i::ping()
{
    return true;
}

