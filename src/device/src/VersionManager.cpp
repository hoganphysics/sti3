#include <sti/device/VersionManager.h>

#include <sti/device/VersionBuildInfo.h>

#include <algorithm>
#include <cctype>
#include <mutex>
#include <sstream>
#include <vector>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

using STI::Device::VersionInfo;
using STI::Device::VersionManager;

namespace {

int parseVersionPart(const std::string& value)
{
    std::string digits;

    for (char ch : value) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            break;
        }
        digits.push_back(ch);
    }

    if (digits.empty()) {
        return 0;
    }

    return std::stoi(digits);
}

void parseVersionString(const std::string& version, int& major, int& minor, int& patch)
{
    major = 0;
    minor = 0;
    patch = 0;

    std::stringstream stream(version);
    std::string part;

    if (std::getline(stream, part, '.')) {
        major = parseVersionPart(part);
    }
    if (std::getline(stream, part, '.')) {
        minor = parseVersionPart(part);
    }
    if (std::getline(stream, part, '.')) {
        patch = parseVersionPart(part);
    }
}

class LocalVersionManager : public VersionManager
{
public:
    LocalVersionManager()
    {
        versions.push_back(STI::Device::getSTILibraryVersion());
    }

    void getVersions(std::vector<VersionInfo>& output) const override
    {
        std::unique_lock<std::mutex> lock(mutex);
        output = versions;
    }

    bool getVersion(const std::string& component, VersionInfo& version) const override
    {
        std::unique_lock<std::mutex> lock(mutex);

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

    VersionInfo getLibraryVersion() const override
    {
        VersionInfo version;
        if (getVersion("sti3", version)) {
            return version;
        }
        return STI::Device::getSTILibraryVersion();
    }

    std::string summary() const override
    {
        std::vector<VersionInfo> snapshot;
        getVersions(snapshot);
        return formatSummary(snapshot);
    }

    bool addVersionInfo(const VersionInfo& version) override
    {
        if (version.component.empty()) {
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex);

        const auto it = std::find_if(versions.begin(), versions.end(),
            [&version](const VersionInfo& info) {
                return info.component == version.component;
            });

        if (it == versions.end()) {
            versions.push_back(version);
        }
        else {
            *it = version;
        }

        return true;
    }

private:
    static std::string formatSummary(const std::vector<VersionInfo>& versions)
    {
        std::stringstream stream;

        for (std::size_t i = 0; i < versions.size(); ++i) {
            if (i > 0) {
                stream << "\n";
            }
            stream << versions[i].toString();
        }

        return stream.str();
    }

    mutable std::mutex mutex;
    std::vector<VersionInfo> versions;
};

} // namespace

VersionInfo::VersionInfo()
    : major(0),
      minor(0),
      patch(0),
      buildNumber(-1),
      gitDirty(false)
{
}

VersionInfo::VersionInfo(const std::string& component, const std::string& version)
    : VersionInfo()
{
    this->component = component;
    this->version = version;
    parseVersionString(version, major, minor, patch);
}

bool VersionInfo::empty() const
{
    return component.empty() && version.empty();
}

std::string VersionInfo::toString() const
{
    std::stringstream stream;

    if (!component.empty()) {
        stream << component;
    }
    else {
        stream << "unknown";
    }

    if (!version.empty()) {
        stream << " " << version;
    }

    if (buildNumber >= 0) {
        stream << " build " << buildNumber;
    }

    if (!buildString.empty()) {
        stream << " (" << buildString << ")";
    }

    if (!gitCommit.empty() && gitCommit != "unknown") {
        stream << " git " << gitCommit;
        if (gitDirty) {
            stream << "-dirty";
        }
    }

    return stream.str();
}

template<class Archive>
void VersionInfo::serialize(Archive& archive)
{
    archive(
        cereal::make_nvp("component", component),
        cereal::make_nvp("version", version),
        cereal::make_nvp("major", major),
        cereal::make_nvp("minor", minor),
        cereal::make_nvp("patch", patch),
        cereal::make_nvp("buildNumber", buildNumber),
        cereal::make_nvp("buildString", buildString),
        cereal::make_nvp("gitCommit", gitCommit),
        cereal::make_nvp("gitDirty", gitDirty),
        cereal::make_nvp("metadata", metadata)
    );
}

std::vector<VersionInfo> VersionManager::getVersions() const
{
    std::vector<VersionInfo> versions;
    getVersions(versions);
    return versions;
}

bool VersionManager::addVersionInfo(const VersionInfo& version)
{
    (void)version;
    return false;
}

VersionInfo STI::Device::getSTILibraryVersion()
{
    VersionInfo version("sti3", STI3_VERSION_STRING);
    version.major = STI3_VERSION_MAJOR;
    version.minor = STI3_VERSION_MINOR;
    version.patch = STI3_VERSION_PATCH;
    version.buildNumber = STI3_BUILD_NUMBER;
    version.buildString = STI3_BUILD_STRING;
    version.gitCommit = STI3_GIT_COMMIT;
    version.gitDirty = (STI3_GIT_DIRTY != 0);
    version.metadata["cmake_build_type"] = STI3_CMAKE_BUILD_TYPE;
    version.metadata["package"] = "stipy";
    return version;
}

std::string STI::Device::getSTILibraryVersionString()
{
    return getSTILibraryVersion().version;
}

std::string STI::Device::getSTILibraryVersionSummary()
{
    return getSTILibraryVersion().toString();
}

std::shared_ptr<VersionManager> STI::Device::makeVersionManager()
{
    return std::make_shared<LocalVersionManager>();
}

template void VersionInfo::serialize<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);
template void VersionInfo::serialize<cereal::XMLInputArchive>(cereal::XMLInputArchive&);
