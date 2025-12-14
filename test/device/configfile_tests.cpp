#include <catch2/catch_test_macros.hpp>

#include <sti/utils/ConfigFile.h>

#include "fileholder_tests_support.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using STI::Utils::ConfigFile;

namespace {

std::filesystem::path writeConfig(const std::filesystem::path& dir, const std::string& name, const std::string& contents) {
    auto path = dir / name;
    std::ofstream ofs(path);
    REQUIRE(ofs.is_open());
    ofs << contents;
    return path;
}

std::string readFileToString(const std::filesystem::path& path) {
    std::ifstream ifs(path);
    REQUIRE(ifs.is_open());
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

} // namespace

TEST_CASE("ConfigFile reports missing files until autocreate is requested", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-missing-");
    auto configPath = tempDir.path / "missing.ini";
    REQUIRE_FALSE(std::filesystem::exists(configPath));

    ConfigFile config(configPath.string());
    CHECK(config.isParsed() == false);
    CHECK(std::filesystem::exists(configPath) == false);
    CHECK(config.getSectionNames().empty());

    config.load(true);
    CHECK(std::filesystem::exists(configPath));
    CHECK(config.isParsed());
}

TEST_CASE("ConfigFile parses sections, comments, and relative subsections", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-parse-");
    const std::string contents = R"(# heading comment
root = base

[Device]
 name = camera
 gain =  5   # inline comment

[.Setup]
offset = 2
= 3
)";
    auto configPath = writeConfig(tempDir.path, "device.ini", contents);

    ConfigFile config(configPath.string());
    REQUIRE(config.isParsed());

    std::string rootValue;
    REQUIRE(config.getParameter("", "root", rootValue));
    CHECK(rootValue == "base");

    std::string deviceName;
    REQUIRE(config.getParameter("Device", "name", deviceName));
    CHECK(deviceName == "camera");

    int gain = 0;
    REQUIRE(config.getParameter("Device", "gain", gain));
    CHECK(gain == 5);

    CHECK(config.includes("Device.Setup", "offset"));
    CHECK(config.isList("Device.Setup", "offset"));
    auto offsets = config.getList("Device.Setup", "offset");
    REQUIRE(offsets.size() == 2);
    CHECK(offsets[0] == "2");
    CHECK(offsets[1] == "3");
}

TEST_CASE("ConfigFile stops parsing on malformed lines", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-malformed-");
    const std::string contents = R"(good = 1
invalid line with no equals
next = 2
)";
    auto configPath = writeConfig(tempDir.path, "bad.ini", contents);

    ConfigFile config(configPath.string());
    CHECK(config.isParsed() == false);

    int good = 0;
    REQUIRE(config.getParameter("good", good));
    CHECK(good == 1);
    CHECK(config.includes("next") == false);
}

TEST_CASE("ConfigFile saves header comments and sections", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-save-");
    auto configPath = tempDir.path / "output.ini";

    ConfigFile config(configPath.string());
    config.setHeader("Config header\nMore info");
    config.set("", "root", "alpha");
    config.set("device", "name", "dev1");
    config.set("device", "mode", "local");
    config.set("device.child", "id", 42);

    config.save();
    REQUIRE(std::filesystem::exists(configPath));

    const std::string expected = "# Config header\n"
                                 "# More info\n"
                                 "\n"
                                 "root = alpha\n"
                                 "\n"
                                 "[device]\n"
                                 "mode = local\n"
                                 "name = dev1\n"
                                 "\n"
                                 "[device.child]\n"
                                 "id = 42\n"
                                 "\n";

    auto written = readFileToString(configPath);
    CHECK(written == expected);
}

TEST_CASE("ConfigFile reload replaces previous content", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-reload-");
    auto firstPath = writeConfig(tempDir.path, "first.ini", "[Old]\nkey = stale\n");
    auto secondPath = writeConfig(tempDir.path, "second.ini", "fresh = 2\n");

    ConfigFile config(firstPath.string());
    REQUIRE(config.isParsed());
    CHECK(config.includes("Old", "key"));

    config.load(secondPath.string());
    REQUIRE(config.isParsed());
    CHECK_FALSE(config.includes("Old", "key"));

    int fresh = 0;
    REQUIRE(config.getParameter("fresh", fresh));
    CHECK(fresh == 2);
}

TEST_CASE("ConfigFile trims section names and ignores trailing comments", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-sections-");
    const std::string contents = R"([Device]   # trailing comment
name = ok
[Device2 missing-bracket
value = 3
[Device3] extra tokens
flag = yes
)";
    auto configPath = writeConfig(tempDir.path, "sections.ini", contents);

    ConfigFile config(configPath.string());
    REQUIRE(config.isParsed());

    std::string name;
    REQUIRE(config.getParameter("Device", "name", name));
    CHECK(name == "ok");

    int value = 0;
    REQUIRE(config.getParameter("Device2 missing-bracket", "value", value));
    CHECK(value == 3);

    std::string flag;
    REQUIRE(config.getParameter("Device3", "flag", flag));
    CHECK(flag == "yes");
}
