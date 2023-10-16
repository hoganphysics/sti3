#include "LocalProfileManager.h"
#include "ProfileTarget.h"

#include <sti/device/DeviceID.h>
#include <sti/device/Device.h>

#include <iostream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>

#include <algorithm>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;


using STI::Device::LocalProfileManager;
using STI::Device::Profile;
using STI::Device::ProfileType;
using STI::Device::ProfileTarget;
using STI::Device::DeviceID;


LocalProfileManager::LocalProfileManager(const STI::Device::DeviceID& deviceID, const std::shared_ptr<STI::Device::DeviceCollection>& collection)
: deviceID(deviceID), deviceCollection(collection)
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

void LocalProfileManager::getProfiles(std::set<std::string>& names) const
{
	profileMap.getKeys(names);
}

bool LocalProfileManager::getProfile(const std::string& name, std::shared_ptr<Profile>& profile) const
{
	return profileMap.get(name, profile) && profile != 0;
}

bool LocalProfileManager::saveProfile(const std::shared_ptr<Profile>& profile)
{
	if (profile == 0) return false;

	if (!profileMap.add(profile->name, profile)) return false;	// failed to add

	//save to disk
	persistenceRefresher();

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
	persistenceRefresher();

	if (saveDependentDevices && deviceCollection != 0) {
		std::set<DeviceID> ids;
		std::shared_ptr<STI::Device::Device> device;
		std::shared_ptr <STI::Device::ProfileManager> manager;

		deviceCollection->getIDs(ids);

		for (auto& id : ids) {
			//skip unless the device declares this device as server
			if (id.getTargetServerID() != deviceID.getID()) continue;

			if (deviceCollection->get(id, device) && device != 0 
				&& device->getProfileManager(manager) && manager != 0) {
				success &= manager->saveCurrentProfile(name, type, true);
			}
		}
	}

	return success;

}


std::string LocalProfileManager::getFilename()
{
	// return "profiles.json";
	return "profiles";
}

void LocalProfileManager::setPersistenceCallback(const std::function<void(void)>& refresher)
{
	persistenceRefresher = refresher;
}

std::string getProfileFilename(const std::string& profileName)
{
	std::stringstream s;

	std::string forbidden = "<>:\"\\|?*";
    s << STI::Utils::replaceChars(profileName, forbidden, "_");
	s << ".json";
	
	return s.str();
}

bool LocalProfileManager::save(const std::string& filename)
{
	STI::Device::Profiles profiles;
	profileMap.getValues(profiles.profiles);

	fs::path baseProfilePath = filename;	//.sti/deviceID/profiles/
    if (!fs::exists(baseProfilePath)) {
        fs::create_directories(baseProfilePath);
    }

	for (auto& profile : profiles.profiles) {
		if (profile == 0) continue;
		fs::path profileFilename = baseProfilePath;
		profileFilename /= getProfileFilename(profile->name);

		std::ofstream file( profileFilename.string() );
		cereal::JSONOutputArchive archive( file );
		archive(*profile);		//save to disk
	}

	return true;
}


void LocalProfileManager::load(const std::string& filename)
{
    fs::path searchPath = filename;
	if (!fs::exists(searchPath)) return;


    for(auto& p : fs::directory_iterator(searchPath)) {
		if (!fs::exists(p)) continue;
        //check that the file has extension .json
        if (p.path().has_extension() && p.path().extension().string() == ".json") {

			std::ifstream file( p.path().string() );
			cereal::JSONInputArchive archive( file );

			auto profile = std::make_shared<STI::Device::Profile>();
			archive(*profile);

			profileMap.add(profile->name, profile);

        }
    }
}

