#include "LocalProfileManager.h"
#include "ProfileTarget.h"

using STI::Device::LocalProfileManager;
using STI::Device::Profile;
using STI::Device::ProfileType;
using STI::Device::ProfileTarget;


LocalProfileManager::LocalProfileManager()
{
}

LocalProfileManager::~LocalProfileManager()
{
}

void LocalProfileManager::addProfileTarget(const std::shared_ptr<ProfileTarget>& target)
{
	if (target == 0) return;

	targets.push_back(target);
}

void LocalProfileManager::getProfiles(std::set<std::string>& names)
{
	profileMap.getKeys(names);
}

bool LocalProfileManager::getProfile(const std::string& name, std::shared_ptr<Profile>& profile)
{
	return profileMap.get(name, profile);
}

bool LocalProfileManager::saveProfile(std::shared_ptr<Profile>& profile)
{
	if (profile == 0) return false;

	if (!profileMap.add(profile->name, profile)) return false;	// failed to add

	//save to disk

	return true;
}


bool LocalProfileManager::loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices)
{
	std::shared_ptr<Profile> cachedProfile;
	std::shared_ptr<Profile> profile;

	if (!profileMap.get(name, cachedProfile)) return false;	// not found
	if (cachedProfile == 0) return false;

	if (cachedProfile->type != type) {
		//deep copy
		profile = std::make_shared<Profile>();
		profile->name = name;
		profile->type = type;
		profile->attributeData = cachedProfile->attributeData;
		profile->channelData = cachedProfile->channelData;
	}
	else {
		profile = cachedProfile;
	}

	bool success = true;

	for (auto& target : targets) {
		success &= target->loadProfile(profile);
	}

	return success;
}

bool LocalProfileManager::saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices)
{
	auto profile = std::make_shared<Profile>();
	profile->name = name;
	profile->type = type;

	bool success = true;

	for (auto& target : targets) {
		success &= target->saveProfile(profile);
	}

	//save to disk

	return success;

}

