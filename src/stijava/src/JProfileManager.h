
#ifndef STI_DEVICE_JPROFILEMANAGER_H
#define STI_DEVICE_JPROFILEMANAGER_H

#include <sti/device/ProfileManager.h>

#include <memory>


namespace STI
{
namespace Device
{

class ProfileManager;


//Java ProfileManager wrapper
class JProfileManager
{
public:
	
	JProfileManager(const std::shared_ptr<STI::Device::ProfileManager>& manager);
	~JProfileManager();

    std::set<std::string> getProfiles() const;
    std::shared_ptr<Profile> getProfile(const std::string& name) const;
    bool saveProfile(const std::shared_ptr<Profile>& profile);

    bool loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices);
    bool saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices);

private:

    std::shared_ptr<STI::Device::ProfileManager> profileManager;

};

} //Device
} //STI

#endif
