
#include "RemoteChannelManager.h"

#include "Convert_Channel.h"

#include "deviceNet.h"
#include "NetworkConvert.h"

using STI::Network::RemoteChannelManager;
using STI::Network::convert;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TChannel;


RemoteChannelManager::RemoteChannelManager(::STI::TNetwork::TChannelManager_ptr channelManager)
	: tChannelManager(STI::TNetwork::TChannelManager::_duplicate(channelManager))
{
}

void RemoteChannelManager::getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels)
{
    if (CORBA::is_nil(tChannelManager)) return;

    try {
		tChannelManager->getChannels();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteChannelManager::getChannel(short channelNumber, std::shared_ptr<STI::Device::Channel>& channel)
{
    if (CORBA::is_nil(tChannelManager)) return false;

    bool success = false;
    STI::TNetwork::TChannel_var tChannel;

	try {
		tChannelManager->getChannel(static_cast<CORBA::Short>(channelNumber), tChannel);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    std::shared_ptr<STI::Device::Channel> localChannel;
    success = convert<TChannel, std::shared_ptr<STI::Device::Channel>>(tChannel, localChannel);

    return success;
}


bool RemoteChannelManager::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
    if (CORBA::is_nil(tChannelManager)) return false;

    bool success = false;

	try {
		success = tChannelManager->writeChannel(static_cast<CORBA::Short>(channel),
                                      convert<MixedValue, TMixedValue>(value));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

bool RemoteChannelManager::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
    if (CORBA::is_nil(tChannelManager)) return false;

    bool success = false;
    STI::TNetwork::TMixedValue_var tData;

	try {
		success = tChannelManager->readChannel(static_cast<CORBA::Short>(channel), 
                                                convert<MixedValue, TMixedValue>(value),
                                                tData);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    if (success) {
        return convert<TMixedValue, MixedValue>(tData, data);
    }

    return false;
}

