#ifndef STI_DEVICE_VERSIONINFO_H
#define STI_DEVICE_VERSIONINFO_H

#include <map>
#include <string>

namespace STI
{
namespace Device
{

class VersionInfo
{
public:
    VersionInfo();
    VersionInfo(const std::string& component, const std::string& version);

    bool empty() const;
    std::string toString() const;

    std::string component;
    std::string version;
    int major;
    int minor;
    int patch;
    int buildNumber;
    std::string buildString;
    std::string gitCommit;
    bool gitDirty;
    std::map<std::string, std::string> metadata;

    template<class Archive>
    void serialize(Archive& archive);
};

} //Device
} //STI

#endif
