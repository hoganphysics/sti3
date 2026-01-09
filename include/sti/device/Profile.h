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

    Profile();
    Profile(const std::string& name);
    virtual ~Profile() {}

    std::string name;
    ProfileType type;
    bool readOnly;
    std::map<std::string, std::string> attributeData;
    std::map<short, STI::Utils::MixedValue> channelData;

    template<class Archive>
	void serialize(Archive& archive);
};

struct Profiles
{
    std::vector<std::shared_ptr<Profile>> profiles;

    template<class Archive>
	void serialize(Archive& archive);
};

} //Device
} //STI

#endif
