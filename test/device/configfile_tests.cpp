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

TEST_CASE("ConfigFile extraction keeps data from nested relative sections", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-extract-");
    const std::string contents = R"([Test]
p1 = Hello
p2 = [1, 2, entry, 4,5]
p3 = Simple

[.STI]
parser = off

[.Persistence]
logdir = home/test

[Numbers]
ui = [h , s,98]

[Numbers.STI]
another = 56
)";
    auto configPath = writeConfig(tempDir.path, "legacy.ini", contents);

    ConfigFile config(configPath.string());
    REQUIRE(config.isParsed());

    CHECK(config.get<std::string>("Test", "p2", "missing").get() == "[1, 2, entry, 4,5]");
    CHECK(config.isList("Test", "p2"));

    config.addToList("Test", "p1", "[95]");
    auto updated = config.getList("Test", "p1");
    REQUIRE(updated.size() == 2);
    CHECK(updated[0] == "Hello");
    CHECK(updated[1] == "[95]");

    auto testConfig = config.extract("Test");
    CHECK(testConfig.get<std::string>("", "p3", "missing").get() == "Simple");
    CHECK(testConfig.get<std::string>("STI", "parser", "missing").get() == "off");
    CHECK(testConfig.get<std::string>("STI.Persistence", "logdir", "missing").get() == "home/test");

    auto numbersConfig = config.extract("Numbers.STI");
    CHECK(numbersConfig.get<int>("", "another", 0).get() == 56);
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

TEST_CASE("ConfigFile keeps comment markers inside quoted values", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-quoted-comments-");
    const std::string contents = R"([Metadata]
color = "#123456" # inline comment
short = '#abc'
description = "uses # in the value"
path = "C:\data#1"
name = camera # still an inline comment
empty = ""
)";
    auto configPath = writeConfig(tempDir.path, "quoted.ini", contents);

    ConfigFile config(configPath.string());
    REQUIRE(config.isParsed());

    CHECK(config.get<std::string>("Metadata", "color", "missing").get() == "#123456");
    CHECK(config.get<std::string>("Metadata", "short", "missing").get() == "#abc");
    CHECK(config.get<std::string>("Metadata", "description", "missing").get() == "uses # in the value");
    CHECK(config.get<std::string>("Metadata", "path", "missing").get() == R"(C:\data#1)");
    CHECK(config.get<std::string>("Metadata", "name", "missing").get() == "camera");
    CHECK(config.get<std::string>("Metadata", "empty", "missing").get() == "");
}

TEST_CASE("ConfigFile supports escaped characters in unquoted values", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-escaped-values-");
    const std::string contents = R"([Metadata]
color = \#123456 # inline comment
quote = \"quoted\"
slash = C:\\data
mixed = prefix\#mid\"quote\"\\tail # inline comment
unknown = C:\data
)";
    auto configPath = writeConfig(tempDir.path, "escaped.ini", contents);

    ConfigFile config(configPath.string());
    REQUIRE(config.isParsed());

    CHECK(config.get<std::string>("Metadata", "color", "missing").get() == "#123456");
    CHECK(config.get<std::string>("Metadata", "quote", "missing").get() == "\"quoted\"");
    CHECK(config.get<std::string>("Metadata", "slash", "missing").get() == R"(C:\data)");
    CHECK(config.get<std::string>("Metadata", "mixed", "missing").get() == R"(prefix#mid"quote"\tail)");
    CHECK(config.get<std::string>("Metadata", "unknown", "missing").get() == R"(C:\data)");
}

TEST_CASE("ConfigFile saves values with comment markers as quoted literals", "[config] [configfile]") {
    fileholder_test_support::TempDir tempDir("configfile-save-quoted-");
    auto configPath = tempDir.path / "output.ini";

    ConfigFile config(configPath.string());
    config.set("Metadata", "Color", "#123456");
    config.set("Metadata", "Description", "display # color");
    config.set("Metadata", "Path", R"(C:\data#1)");
    config.set("Metadata", "SlashOnly", R"(C:\data)");
    config.set("Metadata", "QuoteOnly", R"(say "hello")");
    config.set("Metadata", "QuotedName", "\"literal\"");

    config.save();

    const auto written = readFileToString(configPath);
    CHECK(written.find("Color = \"#123456\"") != std::string::npos);
    CHECK(written.find("Description = \"display # color\"") != std::string::npos);
    CHECK(written.find(R"(Path = "C:\\data#1")") != std::string::npos);
    CHECK(written.find(R"(SlashOnly = "C:\\data")") != std::string::npos);
    CHECK(written.find("QuoteOnly = \"say \\\"hello\\\"\"") != std::string::npos);
    CHECK(written.find("QuotedName = \"\\\"literal\\\"\"") != std::string::npos);

    ConfigFile reloaded(configPath.string());
    REQUIRE(reloaded.isParsed());
    CHECK(reloaded.get<std::string>("Metadata", "Color", "missing").get() == "#123456");
    CHECK(reloaded.get<std::string>("Metadata", "Description", "missing").get() == "display # color");
    CHECK(reloaded.get<std::string>("Metadata", "Path", "missing").get() == R"(C:\data#1)");
    CHECK(reloaded.get<std::string>("Metadata", "SlashOnly", "missing").get() == R"(C:\data)");
    CHECK(reloaded.get<std::string>("Metadata", "QuoteOnly", "missing").get() == R"(say "hello")");
    CHECK(reloaded.get<std::string>("Metadata", "QuotedName", "missing").get() == "\"literal\"");
}
