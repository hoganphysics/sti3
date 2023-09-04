
#include "TProfileManager_i.h"

#include <sti/device/ProfileManager.h>
#include "ORBManager.h"
#include "convert/Convert_Profile.h"

using STI::Network::convert;
using STI::TNetwork::TProfileManager_i;
using STI::Device::Profile;
using STI::TNetwork::TProfile;
using STI::Device::ProfileType;
using STI::TNetwork::TProfileType;


TProfileManager_i::TProfileManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getProfileManager(profileManager);
    }
}

TProfileManager_i::~TProfileManager_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TProfileManager_i::getProfiles(::STI::TNetwork::TStringSeq_out names)
{
    STI::TNetwork::TStringSeq_var tStringSeq_var(new STI::TNetwork::TStringSeq);
    std::set<std::string> localNames;

    names = new STI::TNetwork::TStringSeq();

    if (profileManager != 0) {

        profileManager->getProfiles(localNames);

        std::vector<std::string> namesVec;
        for (auto& name : localNames) {
            namesVec.push_back(name);      //deep copy, but names localNames is short
        }
        convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(namesVec, tStringSeq_var);

        (*names) = tStringSeq_var;
    }
}


::CORBA::Boolean TProfileManager_i::getProfile(const char* name, ::STI::TNetwork::TProfile_out profile)
{
    bool success = false;

    std::shared_ptr<Profile> localProfile;
    profile = new TProfile();

    if (profileManager != 0) {

        success = profileManager->getProfile(name, localProfile) && localProfile != 0;
    }

    if (success) {

        success = convert<Profile, TProfile>(*localProfile, (TProfile&)(*profile));
    }

    return success;
}

::CORBA::Boolean TProfileManager_i::saveProfile(const ::STI::TNetwork::TProfile& profile)
{
    bool success = false;

    auto localProfile = std::make_shared<Profile>();
    success = convert<TProfile, Profile>(profile, *localProfile);

    if (profileManager != 0 && success) {

        success = profileManager->saveProfile(localProfile);
    }

    return success;
}

::CORBA::Boolean TProfileManager_i::loadProfile(const char* name, ::STI::TNetwork::TProfileType type, ::CORBA::Boolean loadDependentDevices)
{
    bool success = false;

    if (profileManager != 0) {

        success = profileManager->loadProfile(name, convert<TProfileType, ProfileType>(type), static_cast<bool>(loadDependentDevices));
    }

    return success;
}

::CORBA::Boolean TProfileManager_i::saveCurrentProfile(const char* name, ::STI::TNetwork::TProfileType type, ::CORBA::Boolean saveDependentDevices)
{
    bool success = false;

    if (profileManager != 0) {

        success = profileManager->saveCurrentProfile(name, convert<TProfileType, ProfileType>(type), static_cast<bool>(saveDependentDevices));
    }

    return success;
}


::CORBA::Boolean TProfileManager_i::ping()
{
	return true;
}
