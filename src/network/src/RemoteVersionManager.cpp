#include "RemoteVersionManager.h"

#include "NetworkConvert.h"

#include <algorithm>
#include <sstream>

using STI::Device::VersionInfo;
using STI::Network::RemoteVersionManager;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;

RemoteVersionManager::RemoteVersionManager(::STI::TNetwork::TDevice_var device)
    : TReferenceHolder<STI::TNetwork::TDevice>(device)
{
}

RemoteVersionManager::~RemoteVersionManager()
{
}

void RemoteVersionManager::getVersions(std::vector<VersionInfo>& versions) const
{
    std::unique_lock<std::mutex> versionLock(versionMutex);

    versions.clear();

    if (isDisabled()) {
        return;
    }

    STI::TNetwork::TVersionInfoSeq_var tVersions(new STI::TNetwork::TVersionInfoSeq);

    try {
        getTRef()->getVersions(tVersions);
        convert<STI::TNetwork::TVersionInfo, VersionInfo>(tVersions.in(), versions);
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }
}

bool RemoteVersionManager::getVersion(const std::string& component, VersionInfo& version) const
{
    std::vector<VersionInfo> versions;
    getVersions(versions);

    const auto it = std::find_if(versions.begin(), versions.end(),
        [&component](const VersionInfo& info) {
            return info.component == component;
        });

    if (it == versions.end()) {
        version = VersionInfo();
        return false;
    }

    version = *it;
    return true;
}

VersionInfo RemoteVersionManager::getLibraryVersion() const
{
    VersionInfo version;
    if (getVersion("sti3", version)) {
        return version;
    }
    return VersionInfo();
}

std::string RemoteVersionManager::summary() const
{
    std::vector<VersionInfo> versions;
    getVersions(versions);

    std::stringstream stream;
    for (std::size_t i = 0; i < versions.size(); ++i) {
        if (i > 0) {
            stream << "\n";
        }
        stream << versions[i].toString();
    }
    return stream.str();
}

bool RemoteVersionManager::addVersionInfo(const VersionInfo& version)
{
    (void)version;
    return false;
}

bool RemoteVersionManager::ping() const
{
    std::unique_lock<std::mutex> versionLock(versionMutex);

    if (isDisabled()) {
        return false;
    }

    try {
        return getTRef()->refresh();
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }

    return false;
}
