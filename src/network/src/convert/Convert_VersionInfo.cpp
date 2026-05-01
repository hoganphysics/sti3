#include "NetworkConvert.h"

#include <sti/device/VersionInfo.h>

using STI::Device::VersionInfo;
using STI::Network::convert;
using STI::TNetwork::TVersionInfo;

template<>
TVersionInfo STI::Network::convert<VersionInfo, TVersionInfo>(const VersionInfo& version)
{
    TVersionInfo tVersion;
    convert<VersionInfo, TVersionInfo>(version, tVersion);
    return tVersion;
}

template<>
VersionInfo STI::Network::convert<TVersionInfo, VersionInfo>(const TVersionInfo& tVersion)
{
    VersionInfo version;
    convert<TVersionInfo, VersionInfo>(tVersion, version);
    return version;
}

template<>
bool STI::Network::convert<VersionInfo, TVersionInfo>(const VersionInfo& version, TVersionInfo& tVersion)
{
    tVersion.componentName = convert<std::string, ::CORBA::String_member>(version.component);
    tVersion.version = convert<std::string, ::CORBA::String_member>(version.version);
    tVersion.major = static_cast<CORBA::Long>(version.major);
    tVersion.minor = static_cast<CORBA::Long>(version.minor);
    tVersion.patch = static_cast<CORBA::Long>(version.patch);
    tVersion.buildNumber = static_cast<CORBA::Long>(version.buildNumber);
    tVersion.buildString = convert<std::string, ::CORBA::String_member>(version.buildString);
    tVersion.gitCommit = convert<std::string, ::CORBA::String_member>(version.gitCommit);
    tVersion.gitDirty = static_cast<CORBA::Boolean>(version.gitDirty);
    convert<std::map<std::string, std::string>, STI::TNetwork::TStringPairSeq>(version.metadata, tVersion.metadata);
    return true;
}

template<>
bool STI::Network::convert<TVersionInfo, VersionInfo>(const TVersionInfo& tVersion, VersionInfo& version)
{
    version.component = convert<::CORBA::String_member, std::string>(tVersion.componentName);
    version.version = convert<::CORBA::String_member, std::string>(tVersion.version);
    version.major = static_cast<int>(tVersion.major);
    version.minor = static_cast<int>(tVersion.minor);
    version.patch = static_cast<int>(tVersion.patch);
    version.buildNumber = static_cast<int>(tVersion.buildNumber);
    version.buildString = convert<::CORBA::String_member, std::string>(tVersion.buildString);
    version.gitCommit = convert<::CORBA::String_member, std::string>(tVersion.gitCommit);
    version.gitDirty = static_cast<bool>(tVersion.gitDirty);
    convert<STI::TNetwork::TStringPairSeq, std::map<std::string, std::string>>(tVersion.metadata, version.metadata);
    return true;
}
