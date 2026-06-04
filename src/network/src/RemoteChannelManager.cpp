#include "RemoteChannelManager.h"

#include "convert/Convert_Channel.h"
#include "DeviceMessageListenerForwarder.h"
#include <sti/device/DeviceMessage.h>
#include "RemoteChannel.h"
#include "generated/deviceNet.h"
#include "NetworkConvert.h"

using STI::Network::RemoteChannelManager;
using STI::Network::convert;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TChannel;
using STI::Device::Channel;
using STI::Device::ChannelUpdateMessage;


RemoteChannelManager::RemoteChannelManager(::STI::TNetwork::TChannelManager_var channelManager,
											const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
											const STI::Device::DeviceID& remoteID)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TChannelManager>(channelManager), 
listenerForwarder(forwarder), remoteID(remoteID)
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

void RemoteChannelManager::setChannelData(const std::shared_ptr<STI::Network::RemoteChannel>& channel)
{
	if (channel != 0) {
		auto channelNumber = channel->getChannelNumber();

		//Don't call to channel->getChannelName() etc, to avoid deadlock (RemoteChannel points back to this class)
		auto chData = channel->getChannelData();
		if (chData != 0) {
			channelData[channelNumber] = chData;
		}
		else {
			channelData[channelNumber] = std::make_shared<ChannelDataTuple>();
		}
	}
}

void RemoteChannelManager::getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (isDisabled()) return;

	STI::TNetwork::TChannelSeq_var tChannels(new STI::TNetwork::TChannelSeq);
	std::vector<std::shared_ptr<RemoteChannel>> remoteChannels;

    try {
		getTRef()->getChannels(tChannels);	//remote call

		if (convert<TChannel, std::shared_ptr<RemoteChannel>>(tChannels, remoteChannels)) {
			
			channels.clear();
			channelData.clear();

			for(auto& rch : remoteChannels) {
				if (rch != 0) {
					rch->attachManager(this);
					setChannelData(rch);
					channels.push_back( std::static_pointer_cast<Channel>(rch) );					
				}
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

    if (isDisabled()) return false;

    bool success = false;
    STI::TNetwork::TChannel_var tChannel;

	try {
		success = getTRef()->getChannel(static_cast<CORBA::Short>(channelNumber), tChannel);	//remote call
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
		setChannelData(remoteChannel);
		channel = remoteChannel;
	}

    return success;
}


bool RemoteChannelManager::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (isDisabled()) return false;

    bool success = false;

	try {
		success = getTRef()->writeChannel(static_cast<CORBA::Short>(channel),
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
    
	if (isDisabled()) return false;

    bool success = false;
    STI::TNetwork::TMixedValue_var tData;

	try {
		success = getTRef()->readChannel(static_cast<CORBA::Short>(channel), 
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

    return success;
}

void RemoteChannelManager::stop()
{
	std::unique_lock<std::mutex> managerLock(managerMutex);
    
	if (isDisabled()) return;

	try {
		getTRef()->stop();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteChannelManager::setChannelName(short channel, const std::string& name)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    if (isDisabled()) return false;

    bool success = false;

	try {
		success = getTRef()->setChannelName(static_cast<CORBA::Short>(channel),
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

	if (isDisabled()) return false;
	
	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
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
	return _getChannelName(channel);
}

std::string RemoteChannelManager::_getChannelName(short channel) const
{
	auto it = channelData.find(channel);

	if (it != channelData.end()) {
		return it->second->name;
	}

	//not found
	std::string name = "";
	return name;
}

STI::Utils::MixedValue RemoteChannelManager::getLastValue(short channel) const
{
	std::unique_lock<std::mutex> managerLock(managerMutex);
	return _getLastValue(channel);
}

STI::Utils::MixedValue RemoteChannelManager::_getLastValue(short channel) const
{
	auto it = channelData.find(channel);

	if (it != channelData.end()) {
		return it->second->value;
	}

	//not found
	STI::Utils::MixedValue empty;
	return empty;
}

STI::Utils::MixedValue RemoteChannelManager::getLastMeasurement(short channel) const
{
	std::unique_lock<std::mutex> managerLock(managerMutex);
	return _getLastMeasurement(channel);
}

STI::Utils::MixedValue RemoteChannelManager::_getLastMeasurement(short channel) const
{
	auto it = channelData.find(channel);

	if (it != channelData.end()) {
		return it->second->measurement;
	}

	STI::Utils::MixedValue empty;
	return empty;
}

void RemoteChannelManager::handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (mess == 0) return;

	if (mess->channelUpdateType == ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue) {
		for (auto& tuple : mess->channelValues) {
			auto ch = channelData[tuple.first];
			if (ch == 0) {
				ch = std::make_shared<ChannelDataTuple>();
				channelData[tuple.first] = ch;
			}
			ch->value = tuple.second;
		}
		for (auto& tuple : mess->measurementValues) {
			auto ch = channelData[tuple.first];
			if (ch == 0) {
				ch = std::make_shared<ChannelDataTuple>();
				channelData[tuple.first] = ch;
			}
			ch->measurement = tuple.second;
		}
	}
	else if (mess->channelUpdateType == ChannelUpdateMessage::ChannelUpdateMessageType::ChannelName) {
		auto ch = channelData[mess->channelNumber];
		if (ch == 0) {
			ch = std::make_shared<ChannelDataTuple>();
			channelData[mess->channelNumber] = ch;
		}
		ch->name = mess->channelName;
	}
}
