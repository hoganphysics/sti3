#ifndef STI_DEVICE_PROFILE_H
#define STI_DEVICE_PROFILE_H

#include <sti/utils/MixedValue.h>

#include <string>
#include <map>


namespace STI
{
namespace Device
{

enum class ProfileType { Attribute, Channel, All };

class Profile
{
public:

    std::string name;
    ProfileType type;
    std::map<std::string, std::string> attributeData;
    std::map<short, STI::Utils::MixedValue> channelData;

};

} //Device
} //STI

#endif
