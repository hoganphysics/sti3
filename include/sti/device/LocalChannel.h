#ifndef STI_DEVICE_LOCALCHANNEL_H
#define STI_DEVICE_LOCALCHANNEL_H

#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>

#include <string>
#include <mutex>


namespace STI
{
namespace Device
{

class ChannelRefreshListener;


class LocalChannel	: public Channel
{
public:

	LocalChannel();
	LocalChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	~LocalChannel();

	short getChannelNumber() const;

	STI::Device::ChannelType getType() const;
	STI::Utils::MixedValueType getInputType() const;
	STI::Utils::MixedValueType getOutputType() const;

	std::string getChannelName() const;

	const STI::Utils::MixedValue getLastValue() const;
	const STI::Utils::MixedValue getLastMeasurement() const;

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;


	void setChannelName(const std::string& name);
	void saveLastValue(const STI::Utils::MixedValue& value);
	void saveLastMeasurement(const STI::Utils::MixedValue& value);
	
	template<typename T>
	void saveLastValue(const T& value)
	{
		STI::Utils::MixedValue mixedVal;
		mixedVal.setValue(value);
		saveLastValue(mixedVal);
	}

	template<typename T>
	void saveLastMeasurement(const T& value)
	{
		STI::Utils::MixedValue mixedVal;
		mixedVal.setValue(value);
		saveLastMeasurement(mixedVal);
	}

	LocalChannel& addMetaData(const std::string& key, const STI::Utils::MixedValue& value);
	LocalChannel& addMetaDataList(const std::string& key, const std::vector<std::string>& values);

	LocalChannel& setColor(const std::string& color);
	LocalChannel& setUnits(const std::string& units);
	LocalChannel& setMeasurementUnits(const std::string& units);
	LocalChannel& setMinValue(const STI::Utils::MixedValue& value);
	LocalChannel& setMaxValue(const STI::Utils::MixedValue& value);
	LocalChannel& setVectorFormat(const std::vector<STI::Utils::MixedValueType>& types);

	LocalChannel& setValueHint(const std::string& hint);
	LocalChannel& setHelp(const std::string& help);

	void addRefreshListener(ChannelRefreshListener* listener);

	// void saveLastInValue(const STI::Utils::MixedValue& value);
	// const STI::Utils::MixedValue getLastInValue() const;

	//getLastValue (unknown is allowed, i.e., XXXXXXX)  Empty is the same as Unknown
	//saveLastValue(const MixedValue&);
	//lastOutValue
	//lastInValue

private:

    void _fireRefreshChannelEvent();
	void _fireRefreshChannelMeasurementEvent();
	void _fireRefreshChannelNameEvent();

	short channelNumber;
	std::string channelName;

	STI::Device::ChannelType type;
	STI::Utils::MixedValueType inputType;
	STI::Utils::MixedValueType outputType;
	
	STI::Utils::MixedValue lastValue;
	STI::Utils::MixedValue lastMeasurement;
//	STI::Utils::MixedValue lastInValue;

	std::vector<ChannelRefreshListener*> listeners;	

	STI::Utils::MetaData metaData;		//usage tip, units, etc

    mutable std::mutex chMutex;

};


} //Device
} //STI

#endif
