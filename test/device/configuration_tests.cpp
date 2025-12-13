#include <catch2/catch_test_macros.hpp>

#include <sti/utils/Configuration.h>

#include <map>
#include <string>
#include <vector>

using STI::Utils::Configuration;

TEST_CASE("Configuration: empty defaults") {
    Configuration config;

    CHECK(config.getSectionNames().empty());
    int value = 0;
    CHECK_FALSE(config.getParameter("missing", value));
    CHECK(config.includes("missing") == false);
    CHECK(config.isList("missing") == false);
    CHECK(config.getList("missing").empty());

    auto result = config.get<int>("missing", 42);
    CHECK(static_cast<int>(result) == 42);
}

TEST_CASE("Configuration: constructor and parameter access") {
    std::map<std::string, std::string> params{{"threshold", "3"}, {"name", "alpha"}};
    Configuration config(params);

    auto names = config.getParameterNames();
    REQUIRE(names.size() == 2);
    CHECK(config.includes("threshold"));
    CHECK(config.includes("name"));

    int threshold = 0;
    REQUIRE(config.getParameter("threshold", threshold));
    CHECK(threshold == 3);

    std::string name;
    REQUIRE(config.getParameter("name", name));
    CHECK(name == "alpha");

    CHECK(config.isList("threshold") == false);
    CHECK(config.getList("threshold") == std::vector<std::string>{"3"});
}

TEST_CASE("Configuration: sections and append merging") {
    Configuration base({{"A", {{"a1", "1"}}}, {"B", {{"b1", "foo"}}}});
    Configuration extra({{"A", {{"a2", "2"}}}, {"C", {{"c1", "bar"}}}});

    base.append(extra);

    auto sections = base.getSectionNames();
    CHECK(sections.size() == 3);

    int a1 = 0;
    int a2 = 0;
    REQUIRE(base.getParameter("A", "a1", a1));
    REQUIRE(base.getParameter("A", "a2", a2));
    CHECK(a1 == 1);
    CHECK(a2 == 2);

    std::string b1;
    REQUIRE(base.getParameter("B", "b1", b1));
    CHECK(b1 == "foo");

    std::string c1;
    REQUIRE(base.getParameter("C", "c1", c1));
    CHECK(c1 == "bar");
}

TEST_CASE("Configuration: list parsing and addToList") {
    Configuration config;
    config.set("nums", "[1, 2 ,3]");

    REQUIRE(config.isList("nums"));
    auto list = config.getList("nums");
    REQUIRE(list.size() == 3);
    CHECK(list[0] == "1");
    CHECK(list[1] == "2");
    CHECK(list[2] == "3");

    config.addToList("nums", 4);
    auto extended = config.getList("nums");
    REQUIRE(extended.size() == 4);
    CHECK(extended[3] == "4");

    config.set("scalar", "solo");
    auto scalarList = config.getList("scalar");
    REQUIRE(scalarList.size() == 1);
    CHECK(scalarList[0] == "solo");
}

TEST_CASE("Configuration: extract and filter preserve/strip prefixes") {
    Configuration config({{"dev", {{"host", "localhost"}}},
                          {"dev.child", {{"id", "child"}}},
                          {"other", {{"keep", "yes"}}}});

    auto extracted = config.extract("dev");
    auto extractedSections = extracted.getSectionNames();
    REQUIRE(extractedSections.size() == 2);
    CHECK(extracted.includes("", "host"));   // prefix stripped for exact match
    CHECK(extracted.includes("child", "id"));

    auto filtered = config.filter("dev");
    auto filteredSections = filtered.getSectionNames();
    REQUIRE(filteredSections.size() == 2);
    CHECK(filtered.includes("dev", "host"));
    CHECK(filtered.includes("dev.child", "id"));
}

TEST_CASE("Configuration: getOrThrow invokes handler on missing keys") {
    Configuration config({{"", {{"present", "5"}}}});
    bool handlerCalled = false;

    auto val = config.getOrThrow<int>("present", [&](const std::string&) { handlerCalled = true; });
    CHECK(static_cast<int>(val) == 5);
    CHECK(handlerCalled == false);

    config.getOrThrow<int>("absent", [&](const std::string& key) {
        handlerCalled = true;
        CHECK(key == "absent");
    });
    CHECK(handlerCalled == true);
}
