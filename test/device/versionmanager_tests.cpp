#include <catch2/catch_test_macros.hpp>

#include <sti/device/VersionInfo.h>
#include <sti/device/VersionManager.h>

#include <memory>
#include <vector>

TEST_CASE("VersionInfo parses semantic version fields", "[version][versioninfo]")
{
    STI::Device::VersionInfo version("device", "1.2.3-beta");

    REQUIRE(version.component == "device");
    REQUIRE(version.version == "1.2.3-beta");
    REQUIRE(version.major == 1);
    REQUIRE(version.minor == 2);
    REQUIRE(version.patch == 3);
    REQUIRE(version.buildNumber == -1);
    REQUIRE_FALSE(version.empty());
}

TEST_CASE("VersionManager reports STI library build information", "[version][versionmanager]")
{
    auto manager = STI::Device::makeVersionManager();
    REQUIRE(manager != nullptr);

    auto libraryVersion = manager->getLibraryVersion();
    REQUIRE(libraryVersion.component == "sti3");
    REQUIRE(libraryVersion.version == STI::Device::getSTILibraryVersionString());
    REQUIRE(libraryVersion.major >= 0);
    REQUIRE(libraryVersion.buildNumber >= 0);
    REQUIRE(libraryVersion.buildString != "placeholder");

    STI::Device::VersionInfo found;
    REQUIRE(manager->getVersion("sti3", found));
    REQUIRE(found.version == libraryVersion.version);

    const auto summary = manager->summary();
    REQUIRE(summary.find("sti3") != std::string::npos);
    REQUIRE(summary.find(libraryVersion.version) != std::string::npos);
}

TEST_CASE("VersionManager appends and replaces device version information", "[version][versionmanager]")
{
    auto manager = STI::Device::makeVersionManager();
    auto initialVersions = manager->getVersions();

    STI::Device::VersionInfo custom("device", "0.1.0");
    custom.buildNumber = 7;
    custom.gitCommit = "abcdef";

    REQUIRE(manager->addVersionInfo(custom));

    STI::Device::VersionInfo found;
    REQUIRE(manager->getVersion("device", found));
    REQUIRE(found.version == "0.1.0");
    REQUIRE(found.buildNumber == 7);

    REQUIRE(manager->getVersions().size() == initialVersions.size() + 1);

    REQUIRE(manager->addVersionInfo(STI::Device::VersionInfo("device", "0.2.0")));
    REQUIRE(manager->getVersion("device", found));
    REQUIRE(found.version == "0.2.0");
    REQUIRE(manager->getVersions().size() == initialVersions.size() + 1);
}
