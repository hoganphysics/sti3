#ifndef STI_DEVICE_PROFILEMANAGER_H
#define STI_DEVICE_PROFILEMANAGER_H

#include <sti/device/Profile.h>

#include <vector>
#include <string>
#include <set>
#include <memory>


namespace STI
{
namespace Device
{

class ProfileManager
{
public:

    virtual ~ProfileManager() {}

    virtual void getProfiles(std::set<std::string>& names) const = 0;
    virtual bool getProfile(const std::string& name, std::shared_ptr<Profile>& profile) const = 0;
    virtual bool saveProfile(const std::shared_ptr<Profile>& profile) = 0;

    virtual bool setReadOnly(const std::string& name, bool readOnly) = 0;

    virtual bool loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices) = 0;
    virtual bool saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices) = 0;
};


} //Device
} //STI

#endif
