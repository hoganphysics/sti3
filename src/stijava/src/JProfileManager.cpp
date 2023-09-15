#include "JProfileManager.h"

#include <sti/device/Profile.h>

using STI::Device::JProfileManager;
using STI::Device::Profile;
using STI::Device::ProfileType;



JProfileManager::JProfileManager(const std::shared_ptr<STI::Device::ProfileManager>& manager)
: profileManager(manager)
{
}

JProfileManager::~JProfileManager()
{
}


std::set<std::string> JProfileManager::getProfiles() const
{
    std::set<std::string> names;
    if (profileManager != 0) {
        profileManager->getProfiles(names);
    }
    return names;
}

std::shared_ptr<Profile> JProfileManager::getProfile(const std::string& name) const
{
    std::shared_ptr<Profile> profile;

    if (profileManager != 0) {
        profileManager->getProfile(name, profile);
    }
    return profile;
}

bool JProfileManager::saveProfile(const std::shared_ptr<Profile>& profile)
{
    if (profileManager != 0) {
        return profileManager->saveProfile(profile);
    }
    return false;
}


bool JProfileManager::loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices)
{
    if (profileManager != 0) {
        return profileManager->loadProfile(name, type, loadDependentDevices);
    }
    return false;
}

bool JProfileManager::saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices)
{
    if (profileManager != 0) {
        return profileManager->saveCurrentProfile(name, type, saveDependentDevices);
    }
    return false;
}

