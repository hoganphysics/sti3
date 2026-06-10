#include <sti/device/LocalChannel.h>
#include "ChannelRefreshListener.h"

using STI::Device::LocalChannel;
using STI::Device::ChannelRefreshListener;


LocalChannel::LocalChannel()
: LocalChannel(0, ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Empty, "")
{
}

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

void LocalChannel::saveLastMeasurement(const STI::Utils::MixedValue& value)
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	lastMeasurement = value;
	_fireRefreshChannelMeasurementEvent();
}

const STI::Utils::MixedValue LocalChannel::getLastMeasurement() const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return lastMeasurement;
}

LocalChannel& LocalChannel::addMetaData(const std::string& key, const STI::Utils::MixedValue& value)
{
    std::unique_lock<std::mutex> channelLock(chMutex);

	metaData.addMetaData(key, value);

	return (*this);
}

LocalChannel& LocalChannel::addMetaDataList(const std::string& key, const std::vector<std::string>& values)
{
    STI::Utils::MixedValue value(values);
    return addMetaData(key, value);
}

LocalChannel& LocalChannel::setColor(const std::string& color)
{
    return addMetaData("color", STI::Utils::MixedValue(color));
}

LocalChannel& LocalChannel::setUnits(const std::string& units)
{
    return addMetaData("units", STI::Utils::MixedValue(units));
}

LocalChannel& LocalChannel::setMeasurementUnits(const std::string& units)
{
    return addMetaData("measurementUnits", STI::Utils::MixedValue(units));
}

LocalChannel& LocalChannel::setMinValue(const STI::Utils::MixedValue& value)
{
    return addMetaData("minValue", value);
}

LocalChannel& LocalChannel::setMaxValue(const STI::Utils::MixedValue& value)
{
    return addMetaData("maxValue", value);
}

LocalChannel& LocalChannel::setVectorFormat(const std::vector<STI::Utils::MixedValueType>& types)
{
    if (getOutputType() != STI::Utils::MixedValueType::Vector) {
        //not a vector channel
        return (*this);
    }

    STI::Utils::MixedValue typeList;
    // typeList.setValue(types);

    // typeList.
    for (const auto& t : types) {
        typeList.addValue(STI::Utils::MixedValue::TypeToString(t));
    }

    return addMetaData("vectorFormat", typeList);
}

LocalChannel& LocalChannel::setValueHint(const std::string& hint)
{
    return addMetaData("valueHint", STI::Utils::MixedValue(hint));
}

LocalChannel& LocalChannel::setHelp(const std::string& help)
{
    return addMetaData("help", STI::Utils::MixedValue(help));
}

const STI::Utils::MixedValue& LocalChannel::getMetaData() const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return metaData.getMetaData();
}

STI::Utils::MixedValue LocalChannel::getMetaData(const std::string& key) const
{
    std::unique_lock<std::mutex> channelLock(chMutex);
	return metaData.getMetaData(key);
}

void LocalChannel::_fireRefreshChannelEvent()
{
    for (auto& listener : listeners) {
        if (listener != 0) {
            listener->handleChannelRefreshEvent(channelNumber, lastValue);
        }
    }
}

void LocalChannel::_fireRefreshChannelMeasurementEvent()
{
    for (auto& listener : listeners) {
        if (listener != 0) {
            listener->handleChannelMeasurementRefreshEvent(channelNumber, lastMeasurement);
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
