
#include "convert/Convert_Profile.h"
#include "Convert_Attribute.h"
#include "Convert_DeviceMessage.h"


using STI::Network::convert;
using STI::Device::Profile;
using STI::TNetwork::TProfile;
using STI::Device::ProfileType;
using STI::TNetwork::TProfileType;
using STI::TNetwork::TAttributeTupleSeq;
using STI::TNetwork::TChannelUpdateTupleSeq;
using STI::Utils::MixedValue;


//ProfileType
template<>
ProfileType STI::Network::convert<TProfileType, ProfileType>(const TProfileType& tType)
{
    ProfileType type;

    switch (tType) 
    {
    case TProfileType::ProfileAttribute:
        type = ProfileType::Attribute;
        break;
    case TProfileType::ProfileChannel:
        type = ProfileType::Channel;
        break;
    case TProfileType::ProfileAll:
        type = ProfileType::All;
        break;
    default:
        type = ProfileType::All;
        break;
    }
    return type;
}

template<>
TProfileType STI::Network::convert<ProfileType, TProfileType>(const ProfileType& type)
{
    TProfileType tType;

    switch (type)
    {
    case ProfileType::Attribute:
        tType = TProfileType::ProfileAttribute;
        break;
    case ProfileType::Channel:
        tType = TProfileType::ProfileChannel;
        break;
        case ProfileType::All:
        tType = TProfileType::ProfileAll;
        break;
    default:
        tType = TProfileType::ProfileAll;
        break;
    }
    return tType;
}


//Profile
template<>
bool STI::Network::convert<TProfile, Profile>(const TProfile& tProfile, Profile& profile)
{
    profile.name = convert<::CORBA::String_member, std::string>(tProfile.name);
    profile.type = convert<TProfileType, ProfileType>(tProfile.type);

    convert<TAttributeTupleSeq, std::map<std::string, std::string>>(tProfile.attributeData, profile.attributeData);
    convert<TChannelUpdateTupleSeq, std::map<short, MixedValue>>(tProfile.channelData, profile.channelData);

    return true;
}

template<>
bool STI::Network::convert<Profile, TProfile>(const Profile& profile, TProfile& tProfile)
{
    tProfile.name = convert<std::string, ::CORBA::String_member>(profile.name);
    tProfile.type = convert<ProfileType, TProfileType>(profile.type);

    convert<std::map<std::string, std::string>, TAttributeTupleSeq>(profile.attributeData, tProfile.attributeData);
    convert<std::map<short, MixedValue>, TChannelUpdateTupleSeq>(profile.channelData, tProfile.channelData);

    return true;
}
