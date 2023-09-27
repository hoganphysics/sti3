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


class LocalChannel	: public Channel //: public EventEmitter<ChannelEvent>
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

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;


	void setChannelName(const std::string& name);
	void saveLastValue(const STI::Utils::MixedValue& value);
	
	template<typename T>
	void saveLastValue(const T& value)
	{
		STI::Utils::MixedValue mixedVal;
		mixedVal.setValue(value);
		saveLastValue(mixedVal);
	}

	LocalChannel& addMetaData(const std::string& key, const STI::Utils::MixedValue& value);
	LocalChannel& addMetaDataList(const std::string& key, const std::vector<std::string>& values);

	void addRefreshListener(ChannelRefreshListener* listener);

	// void saveLastInValue(const STI::Utils::MixedValue& value);
	// const STI::Utils::MixedValue getLastInValue() const;

//	std::string usageTip;

	//getLastValue (unknown is allowed, i.e., XXXXXXX)  Is Empty the same as Unknown?  I think so!
	//saveLastValue(const MixedValue&);
	//lastOutValue
	//lastInValue

private:

    void _fireRefreshChannelEvent();
	void _fireRefreshChannelNameEvent();

	short channelNumber;
	std::string channelName;

	STI::Device::ChannelType type;
	STI::Utils::MixedValueType inputType;
	STI::Utils::MixedValueType outputType;
	
	STI::Utils::MixedValue lastValue;
//	STI::Utils::MixedValue lastInValue;

	std::vector<ChannelRefreshListener*> listeners;	

	// STI::Utils::MixedValue metaData;	//usage tip, units, etc
	STI::Utils::MetaData metaData;		//usage tip, units, etc

    mutable std::mutex chMutex;

};


} //Device
} //STI

#endif

