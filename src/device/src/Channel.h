#ifndef STI_DEVICE_CHANNEL_H
#define STI_DEVICE_CHANNEL_H

#include "fwd/Channel_fwd.h"
#include "fwd/MixedValue_fwd.h"

#include <string>

namespace STI
{
namespace Device
{

//enum MixedValueType { Boolean, Int, Double, String, Vector, Empty }; File, Image, Any
//enum TValue { ValueNumber, ValueString, ValueVector, ValueNone };
//enum TData { DataBoolean, DataOctet, DataLong, DataDouble, DataString, DataPicture, DataVector, DataFile, DataNone };
enum TChannelType { Output, Input };

class Channel	//: public EventEmitter<ChannelEvent>
{
public:

	Channel(unsigned short channelNumber, STI::Device::TChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	~Channel();

	//MixedValue lastValue;
	short channelNumber;

	std::string usageTip;

	STI::Device::TChannelType type;
	STI::Utils::MixedValueType inputType;
	STI::Utils::MixedValueType outputType;

	void setChannelName(const std::string& name);
	std::string getChannelName() const;

	//getLastValue (unknown is allowed, i.e., XXXXXXX)  Is Empty the same as Unknown?  I think so!
	//saveLastValue(const MixedValue&);
	//lastOutValue
	//lastInValue

private:

	std::string channelName;

};


} //Device
} //STI

#endif

