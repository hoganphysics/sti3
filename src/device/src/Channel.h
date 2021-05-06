#ifndef STI_DEVICE_CHANNEL_H
#define STI_DEVICE_CHANNEL_H

#include "fwd/Channel_fwd.h"
#include "MixedValue.h"

#include <string>
#include <mutex>

namespace STI
{
namespace Device
{

//enum MixedValueType { Boolean, Int, Double, String, Vector, Empty }; File, Image, Any
//enum TValue { ValueNumber, ValueString, ValueVector, ValueNone };
//enum TData { DataBoolean, DataOctet, DataLong, DataDouble, DataString, DataPicture, DataVector, DataFile, DataNone };



class Channel
{
public:

	virtual ~Channel() {}

	virtual short getChannelNumber() const = 0;

	virtual STI::Device::ChannelType getType() const = 0;
	virtual STI::Utils::MixedValueType getInputType() const = 0;
	virtual STI::Utils::MixedValueType getOutputType() const = 0;

	virtual void setChannelName(const std::string& name) = 0;
	virtual std::string getChannelName() const = 0;

	virtual void saveLastValue(const STI::Utils::MixedValue& value) = 0;
	virtual const STI::Utils::MixedValue getLastValue() const = 0;

	//Metadata can be used for GUI layout, tooltips, units, etc.
	virtual const STI::Utils::MixedValue& getMetaData() const = 0;
	virtual STI::Utils::MixedValue getMetaData(const std::string& key) const = 0;

	static std::string typeToString(const STI::Device::ChannelType& type)
	{
		std::string name = "Unknown";

		switch (type) {
		case ChannelType::Input:
			name = "Input";
			break;
		case ChannelType::Output:
			name = "Output";
			break;
		}
		return name;
	}

};


} //Device
} //STI

#endif

