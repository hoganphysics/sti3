#ifndef STI_DEVICE_LOCALDEVICEHELPERS_H
#define STI_DEVICE_LOCALDEVICEHELPERS_H

#include <sti/fwd/MixedValue_fwd.h>
#include <sti/utils/Configuration.h>

#include <cstdint>
#include <string>

namespace STI
{
namespace Device
{

class LocalDevice;

void throwLocalDeviceConstructorConfigError(const std::string& key);
std::uintmax_t getLogMaxFileSizeBytes(const STI::Utils::Configuration& config);
STI::Utils::Configuration makeDeviceScopedConfig(const STI::Utils::Configuration& config, const std::string& section);
void applyConfiguredMetaData(LocalDevice& device, const STI::Utils::Configuration& config);
bool mixedValueMatchesType(const STI::Utils::MixedValue& value, STI::Utils::MixedValueType expectedType);

} //Device
} //STI

#endif
