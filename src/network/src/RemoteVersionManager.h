#ifndef STI_NETWORK_REMOTEVERSIONMANAGER_H
#define STI_NETWORK_REMOTEVERSIONMANAGER_H

#include "generated/deviceNet.h"
#include "TReferenceHolder.h"

#include <sti/device/VersionManager.h>

#include <mutex>
#include <string>
#include <vector>

namespace STI
{
namespace Network
{

class RemoteVersionManager : public STI::Device::VersionManager,
                             public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDevice>
{
public:
    RemoteVersionManager(::STI::TNetwork::TDevice_var device);
    ~RemoteVersionManager();

    void getVersions(std::vector<STI::Device::VersionInfo>& versions) const override;
    bool getVersion(const std::string& component, STI::Device::VersionInfo& version) const override;
    STI::Device::VersionInfo getLibraryVersion() const override;
    std::string summary() const override;
    bool addVersionInfo(const STI::Device::VersionInfo& version) override;

    bool ping() const;

private:
    mutable std::mutex versionMutex;
};

} //Network
} //STI

#endif
