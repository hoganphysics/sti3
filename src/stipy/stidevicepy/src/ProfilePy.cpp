#include "ProfilePy.h"

using STI::Python::ProfilePy;
using STI::Device::Profile;
using STI::Python::MixedValuePy;
using STI::Utils::MixedValue;

ProfilePy::ProfilePy()
: ProfilePy("")
{
}

ProfilePy::ProfilePy(const std::string& name)
: name(name), type(STI::Device::ProfileType::All), readOnly(false)
{
}

ProfilePy::ProfilePy(const Profile& profile)
{
	name = profile.name;
	type = profile.type;
	
	readOnly = profile.readOnly;

	attributeData = profile.attributeData;

	for (auto& ch : profile.channelData) {
		//wrap with python type
		channelData[ch.first].setValue(ch.second);
	}
}

std::shared_ptr<STI::Device::Profile> ProfilePy::toProfile()
{
	auto profile = std::make_shared<Profile>();
	profile->name = name;
	profile->type = type;
	profile->readOnly = readOnly;
	profile->attributeData = attributeData;

	for (auto& ch : channelData) {
		//convert back to STI::Utils::MixedValue
		const MixedValue& value = static_cast<const MixedValue&>(ch.second);
		profile->channelData[ch.first].setValue(value);
	}

	return profile;
}
