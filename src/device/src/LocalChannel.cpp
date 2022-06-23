
#include <sti/device/LocalChannel.h>
#include "ChannelRefreshListener.h"

using STI::Device::LocalChannel;
using STI::Device::ChannelRefreshListener;


LocalChannel::LocalChannel(unsigned short channelNumber, STI::Device::ChannelType type,
	STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
	: channelNumber(channelNumber), type(type), inputType(inputType), outputType(outputType), channelName(defaultName)
{
}

LocalChannel::~LocalChannel()
{
}

void LocalChannel::addRefreshListener(ChannelRefreshListener* listener)
{
    std::unique_lock<std::mutex> channelLock(chMutex);
    listeners.push_back(listener);
}

void LocalChannel::setChannelName(const std::string& name)
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	channelName = name;
	_fireRefreshChannelNameEvent();
}

std::string LocalChannel::getChannelName() const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return channelName;
}

short LocalChannel::getChannelNumber() const
{
	return channelNumber;
}

STI::Device::ChannelType LocalChannel::getType() const
{
	return type;
}

STI::Utils::MixedValueType LocalChannel::getInputType() const
{
	return inputType;
}

STI::Utils::MixedValueType LocalChannel::getOutputType() const
{
	return outputType;
}


void LocalChannel::saveLastValue(const STI::Utils::MixedValue& value)
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	lastValue = value;
	_fireRefreshChannelEvent();
}

const STI::Utils::MixedValue LocalChannel::getLastValue() const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return lastValue;
}

LocalChannel& LocalChannel::addMetaData(const std::string& key, const STI::Utils::MixedValue& value)
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	
	STI::Utils::MixedValue labeledData;
	labeledData.addValue(key);
	labeledData.addValue(value);

	metaData.addValue(labeledData);

	return (*this);
}

const STI::Utils::MixedValue& LocalChannel::getMetaData() const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return metaData;
}

STI::Utils::MixedValue LocalChannel::getMetaData(const std::string& key) const
{
    std::unique_lock<std::mutex> channelLock(chMutex);

	STI::Utils::MixedValue data;	//empty

	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		const STI::Utils::MixedValueVector& labeledData = tuple.getVector();
		if (labeledData.size() == 2 && labeledData.at(0).getString().compare(key) == 0) {
			data = labeledData.at(1);
			break;
		}
	}

	return data;
}

void LocalChannel::_fireRefreshChannelEvent()
{
    for (auto& listener : listeners) {
        if (listener != 0) {
            listener->handleChannelRefreshEvent(channelNumber, lastValue);
        }
    }
}

void LocalChannel::_fireRefreshChannelNameEvent()
{
    for (auto& listener : listeners) {
        if (listener != 0) {
            listener->handleChannelNameRefreshEvent(channelNumber, channelName);
        }
    }
}

