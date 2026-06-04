#ifndef STI_DEVICE_CHANNELSTATE_H
#define STI_DEVICE_CHANNELSTATE_H

#include <sti/utils/MixedValue.h>

namespace STI
{
namespace Device
{

inline bool containsHeavyChannelStateValue(const STI::Utils::MixedValue& value)
{
	switch (value.getType()) {
	case STI::Utils::MixedValueType::Binary:
	case STI::Utils::MixedValueType::Image:
		return true;
	case STI::Utils::MixedValueType::Vector:
		for (const auto& item : value.getVector()) {
			if (containsHeavyChannelStateValue(item)) {
				return true;
			}
		}
		return false;
	default:
		return false;
	}
}

inline STI::Utils::MixedValue makeLightweightChannelMeasurementValue(const STI::Utils::MixedValue& value)
{
	if (containsHeavyChannelStateValue(value)) {
		return STI::Utils::MixedValue();
	}
	return value;
}

} //Device
} //STI

#endif
