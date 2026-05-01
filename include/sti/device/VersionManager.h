#ifndef STI_DEVICE_VERSIONMANAGER_H
#define STI_DEVICE_VERSIONMANAGER_H

#include <sti/device/VersionInfo.h>

#include <memory>
#include <string>
#include <vector>

namespace STI
{
namespace Device
{

class VersionManager
{
public:
    virtual ~VersionManager() {}

    virtual void getVersions(std::vector<VersionInfo>& versions) const = 0;
    std::vector<VersionInfo> getVersions() const;

    virtual bool getVersion(const std::string& component, VersionInfo& version) const = 0;
    virtual VersionInfo getLibraryVersion() const = 0;
    virtual std::string summary() const = 0;

    virtual bool addVersionInfo(const VersionInfo& version);
};

VersionInfo getSTILibraryVersion();
std::string getSTILibraryVersionString();
std::string getSTILibraryVersionSummary();
std::shared_ptr<VersionManager> makeVersionManager();

} //Device
} //STI

#endif
