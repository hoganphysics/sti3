#include <sti/device/Profile.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/map.hpp>


using STI::Device::Profile;
using STI::Device::Profiles;

Profile::Profile()
: name(""), type(ProfileType::All), readOnly(false)
{
}

Profile::Profile(const std::string& name)
: name(name), type(ProfileType::All), readOnly(false)
{
}

template<class Archive>
void Profile::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", name), 
		cereal::make_nvp("type", type), 
		cereal::make_nvp("attributeData", attributeData),
		cereal::make_nvp("channelData", channelData)
		);
}

template void Profile::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void Profile::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void Profile::serialize<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& );
template void Profile::serialize<cereal::JSONInputArchive>( cereal::JSONInputArchive& );


template<class Archive>
void Profiles::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("profiles", profiles)
		);
}

template void Profiles::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void Profiles::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void Profiles::serialize<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& );
template void Profiles::serialize<cereal::JSONInputArchive>( cereal::JSONInputArchive& );
