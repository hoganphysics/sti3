#ifndef STI_TNETWORK_TPROFILEMANAGER_I_H
#define STI_TNETWORK_TPROFILEMANAGER_I_H

#include <sti/device/ProfileManager.h>
#include <sti/device/Device.h>
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TProfileManager_i : public POA_STI::TNetwork::TProfileManager, 
                          public PortableServer::RefCountServantBase
{
public:

    TProfileManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TProfileManager_i();
    
    void getProfiles(::STI::TNetwork::TStringSeq_out names);
    ::CORBA::Boolean getProfile(const char* name, ::STI::TNetwork::TProfile_out profile);
    ::CORBA::Boolean saveProfile(const ::STI::TNetwork::TProfile& profile);
    ::CORBA::Boolean setReadOnly(const char* name, ::CORBA::Boolean readOnlyProfile);
    ::CORBA::Boolean loadProfile(const char* name, ::STI::TNetwork::TProfileType type, ::CORBA::Boolean loadDependentDevices);
    ::CORBA::Boolean saveCurrentProfile(const char* name, ::STI::TNetwork::TProfileType type, ::CORBA::Boolean saveDependentDevices);
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::ProfileManager> profileManager;

};


} //TNetwork
} //STI

#endif

