#include "LocalDeviceHelpers.h"

#include <sti/LocalDevice.h>
#include <sti/utils/MixedValue.h>

#include "LocalLogManager.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace
{

std::string toLowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

} // namespace

namespace STI
{
namespace Device
{

void throwLocalDeviceConstructorConfigError(const std::string& key)
{
    std::stringstream message;
    message << "LocalDevice constructor error: ";
    message << "Required parameter '" << key << "' was not found in the Configuration.";
    throw std::runtime_error(message.str());
}

std::uintmax_t getLogMaxFileSizeBytes(const STI::Utils::Configuration& config)
{
    auto maxFileSizeBytes = config.get<std::uintmax_t>(
        "Logs",
        "Max File Size Bytes",
        config.get<std::uintmax_t>("Logs", "Max File Size", LocalLogManager::DefaultMaxLogFileSizeBytes).get()).get();

    if (maxFileSizeBytes == 0) {
        return LocalLogManager::DefaultMaxLogFileSizeBytes;
    }

    return maxFileSizeBytes;
}

STI::Utils::Configuration makeDeviceScopedConfig(const STI::Utils::Configuration& config, const std::string& section)
{
    if (section.empty()) {
        return config;
    }

    auto deviceConfig = config.extract(section);
    deviceConfig.append(config);
    return deviceConfig;
}

void applyConfiguredMetaData(LocalDevice& device, const STI::Utils::Configuration& config)
{
    for (const auto& [key, value] : config.getParameters("Metadata")) {
        const auto normalizedKey = toLowerAscii(key);

        if (normalizedKey == "color") {
            device.setColor(value);
        }
        else if (normalizedKey == "description") {
            device.setDescription(value);
        }
        else if (normalizedKey == "help") {
            device.setHelp(value);
        }
        else {
            device.addMetaData(key, STI::Utils::MixedValue(value));
        }
    }
}

bool mixedValueMatchesType(const STI::Utils::MixedValue& value, STI::Utils::MixedValueType expectedType)
{
    return expectedType == STI::Utils::MixedValueType::Any || value.isType(expectedType);
}

} //Device
} //STI
