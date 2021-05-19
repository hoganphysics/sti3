
#include "RemoteChannelManager.h"

#include "Convert_Channel.h"
#include "DeviceMessageListenerForwarder.h"
#include "DeviceMessage.h"
#include "RemoteChannel.h"
#include "deviceNet.h"
#include "NetworkConvert.h"

using STI::Network::RemoteChannelManager;
using STI::Network::convert;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TChannel;
using STI::Device::Channel;
using STI::Device::ChannelUpdateMessage;


RemoteChannelManager::RemoteChannelManager(::STI::TNetwork::TChannelManager_ptr channelManager,
											const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
											const STI::Device::DeviceID& remoteID)
	: tChannelManager(STI::TNetwork::TChannelManager::_duplicate(channelManager)), listenerForwarder(forwarder), remoteID(remoteID)
{

	//Message listener for channel update messages
	std::shared_ptr<STI::Device::DeviceMessageListener<STI::Device::ChannelUpdateMessage>> listener;
	listener = std::make_shared<ChannelUpdater>(this);
	
	listenerID.name = "RemoteChannelManager::ChannelUpdate";
	listenerID.type = STI::Device::DeviceMessageType::ChannelUpdate;

	if (listenerForwarder != 0) {
		listenerForwarder->addListener(remoteID, listenerID, listener);		//Listen to events gemerated by remoteID
	}

	//Initialize channel data
	std::vector<std::shared_ptr<STI::Device::Channel>> channels;
	getChannels(channels);
}

RemoteChannelManager::~RemoteChannelManager()
{
	if (listenerForwarder != 0) {
		listenerForwarder->removeListener(remoteID, listenerID);		
	}
}

void RemoteChannelManager::setChannelData(const std::shared_ptr<STI::Device::Channel>& channel)
{
	if (channel != 0) {
		channelData[channel->getChannelNumber()].name = channel->getChannelName();
		channelData[channel->getChannelNumber()].value = channel->getLastValue();			
	}
}

void RemoteChannelManager::getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (CORBA::is_nil(tChannelManager)) return;

	STI::TNetwork::TChannelSeq_var tChannels(new STI::TNetwork::TChannelSeq);
	std::vector<std::shared_ptr<RemoteChannel>> remoteChannels;

    try {
		tChannelManager->getChannels(tChannels);	//remote call

		if (convert<TChannel, std::shared_ptr<RemoteChannel>>(tChannels, remoteChannels)) {
			
			channels.clear();
			for(auto& rch : remoteChannels) {
				if (rch != 0) {
					rch->attachManager(this);
					channels.push_back( std::static_pointer_cast<Channel>(rch) );					
				}
			}

			channelData.clear();
			for(auto& ch : channels) {
				setChannelData(ch);
			}
		}
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
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (CORBA::is_nil(tChannelManager)) return false;

    bool success = false;
    STI::TNetwork::TChannel_var tChannel;

	try {
		success = tChannelManager->getChannel(static_cast<CORBA::Short>(channelNumber), tChannel);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	std::shared_ptr<RemoteChannel> remoteChannel;

    if (success && convert<TChannel, std::shared_ptr<RemoteChannel>>(tChannel, remoteChannel)) {
		remoteChannel->attachManager(this);
		setChannelData(channel);
	}

    return success;
}


bool RemoteChannelManager::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

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
	std::unique_lock<std::mutex> managerLock(managerMutex);
    
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

bool RemoteChannelManager::setChannelName(short channel, const std::string& name)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (CORBA::is_nil(tChannelManager)) return false;

    bool success = false;

	try {
		success = tChannelManager->setChannelName(static_cast<CORBA::Short>(channel),
                                      convert<std::string, CORBA::String_member>(name));	//remote call
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

bool RemoteChannelManager::ping() const
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (CORBA::is_nil(tChannelManager)) return false;
	
	bool success = false;

	try {
		success = tChannelManager->ping();	//remote call
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

std::string RemoteChannelManager::getChannelName(short channel) const
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	auto it = channelData.find(channel);

	if (it != channelData.end()) {
		return it->second.name;
	}

	//not found
	std::string name = "";
	return name;
}

STI::Utils::MixedValue RemoteChannelManager::getLastValue(short channel) const
{
	std::unique_lock<std::mutex> managerLock(managerMutex);
	
	auto it = channelData.find(channel);

	if (it != channelData.end()) {
		return it->second.value;
	}

	//not found
	STI::Utils::MixedValue empty;
	return empty;
}

void RemoteChannelManager::handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (mess == 0) return;

	if (mess->channelUpdateType == ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue) {
		for (auto& tuple : mess->channelValues) {
			channelData[tuple.first].value = tuple.second;
		}
	}
	else if (mess->channelUpdateType == ChannelUpdateMessage::ChannelUpdateMessageType::ChannelName) {
		channelData[mess->channelNumber].name = mess->channelName;
	}
}

