#ifndef STI_DEVICE_CHANNEL_TESTS_SUPPORT_H
#define STI_DEVICE_CHANNEL_TESTS_SUPPORT_H

#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>

#include <string>

class TestChannel : public STI::Device::Channel {
public:
    TestChannel(short channelNumber,
                STI::Device::ChannelType type,
                STI::Utils::MixedValueType inputType,
                STI::Utils::MixedValueType outputType,
                const std::string& name)
        : channelNumber(channelNumber), type(type), inputType(inputType), outputType(outputType), channelName(name) {}

    short getChannelNumber() const override { return channelNumber; }
    STI::Device::ChannelType getType() const override { return type; }
    STI::Utils::MixedValueType getInputType() const override { return inputType; }
    STI::Utils::MixedValueType getOutputType() const override { return outputType; }

    void setChannelName(const std::string& name) override { channelName = name; }
    std::string getChannelName() const override { return channelName; }

    void saveLastValue(const STI::Utils::MixedValue& value) override { lastValue = value; }
    const STI::Utils::MixedValue getLastValue() const override { return lastValue; }

    const STI::Utils::MixedValue& getMetaData() const override { return metaData.getMetaData(); }
    STI::Utils::MixedValue getMetaData(const std::string& key) const override { return metaData.getMetaData(key); }

    void addMeta(const std::string& key, const STI::Utils::MixedValue& value) { metaData.addMetaData(key, value); }

private:
    short channelNumber;
    STI::Device::ChannelType type;
    STI::Utils::MixedValueType inputType;
    STI::Utils::MixedValueType outputType;
    std::string channelName;
    STI::Utils::MixedValue lastValue;
    STI::Utils::MetaData metaData;
};

#endif
