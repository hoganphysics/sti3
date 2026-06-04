#ifndef STI_DEVICE_REMOTECHANNEL_H
#define STI_DEVICE_REMOTECHANNEL_H

#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>

#include <string>
#include <memory>


namespace STI
{
namespace Network
{

class RemoteChannelManager;
struct ChannelDataTuple;


class RemoteChannel : public STI::Device::Channel
{
public:

	RemoteChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, 
        const std::string& channelName, const STI::Utils::MixedValue& lastValue, 
		const STI::Utils::MixedValue& lastMeasurement, const STI::Utils::MixedValue& metaData);

    void attachManager(RemoteChannelManager* manager);

	short getChannelNumber() const;

	STI::Device::ChannelType getType() const;
	STI::Utils::MixedValueType getInputType() const;
	STI::Utils::MixedValueType getOutputType() const;

	void setChannelName(const std::string& name);
	std::string getChannelName() const;

	void saveLastValue(const STI::Utils::MixedValue& value);
	const STI::Utils::MixedValue getLastValue() const;
	void saveLastMeasurement(const STI::Utils::MixedValue& value);
	const STI::Utils::MixedValue getLastMeasurement() const;

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;

    //For updates from push events
    // void updateChannelName(const std::string& name);

private:

	friend RemoteChannelManager;
	std::shared_ptr<ChannelDataTuple> getChannelData() const;

    unsigned short channelNumber_;
    STI::Device::ChannelType type_;
	STI::Utils::MixedValueType inputType_;
    STI::Utils::MixedValueType outputType_;
    // std::string channelName_;
    STI::Utils::MetaData metaData_;

    std::shared_ptr<ChannelDataTuple> channelData;

    RemoteChannelManager* remoteManager;


};


} //Network
} //STI

#endif
