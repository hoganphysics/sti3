#ifndef STI_DEVICE_LOCALPROFILEMANAGER_H
#define STI_DEVICE_LOCALPROFILEMANAGER_H

#include <sti/device/DeviceCollection.h>
#include <sti/device/ProfileManager.h>
#include <sti/utils/SynchronizedMap.h>

#include "PersistenceTarget.h"

#include <vector>
#include <string>
#include <set>
#include <memory>
#include <functional>


namespace STI
{
namespace Device
{

class ProfileTarget;

class LocalProfileManager : public ProfileManager,
                            public PersistenceTarget
{
public:

    LocalProfileManager(const STI::Device::DeviceID& deviceID, const std::shared_ptr<DeviceCollection>& collection);
    ~LocalProfileManager();

    void addProfileTarget(const std::shared_ptr<ProfileTarget>& target);

    void getProfiles(std::set<std::string>& names) const;
    bool getProfile(const std::string& name, std::shared_ptr<Profile>& profile) const;
    bool saveProfile(const std::shared_ptr<Profile>& profile);

    bool loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices);
    bool saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices);

    bool loadProfile(const std::string& name);
    bool saveCurrentProfile(const std::string& name);

private:

    //PersistenceTarget
    std::string getFilename();
    void setPersistenceCallback(const std::function<void(void)>& refresher);
    bool save(const std::string& filename);
    void load(const std::string& filename);

    std::string getDependentProfileName(const std::string& name) const;

    std::function<void(void)> persistenceRefresher;

    STI::Utils::SynchronizedMap<std::string, std::shared_ptr<Profile>> profileMap;

    std::vector<std::shared_ptr<ProfileTarget>> targets;

    STI::Device::DeviceID deviceID;
    std::shared_ptr<DeviceCollection> deviceCollection;
};


} //Device
} //STI

#endif
