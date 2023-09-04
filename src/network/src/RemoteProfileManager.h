#ifndef STI_NETWORK_REMOTEPROFILEMANAGER_H
#define STI_NETWORK_REMOTEPROFILEMANAGER_H

#include "generated/deviceNet.h"
#include <sti/device/ProfileManager.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <string>
#include <set>


namespace STI
{
namespace Network
{

class RemoteProfileManager : public STI::Device::ProfileManager,
							 public STI::TNetwork::TReferenceHolder<STI::TNetwork::TProfileManager>	//mixin
{
public:

    RemoteProfileManager(::STI::TNetwork::TProfileManager_ptr manager);
    ~RemoteProfileManager();

    void getProfiles(std::set<std::string>& names) const;
    bool getProfile(const std::string& name, std::shared_ptr<STI::Device::Profile>& profile) const;
    bool saveProfile(const std::shared_ptr<STI::Device::Profile>& profile);

    bool loadProfile(const std::string& name, const STI::Device::ProfileType& type, bool loadDependentDevices);
    bool saveCurrentProfile(const std::string& name, const STI::Device::ProfileType& type, bool saveDependentDevices);

    bool ping() const;

private:

	mutable std::mutex profileMutex;
};


} //Network
} //STI


#endif
