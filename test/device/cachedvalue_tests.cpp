#include <catch2/catch_test_macros.hpp>

#include <sti/utils/CachedValue.h>

#include <string>

using STI::Utils::CachedValue;

TEST_CASE("CachedValue: defaults not cached") {
    CachedValue<int> cache;

    int out = 7;
    CHECK(cache.isCached() == false);
    CHECK(cache.getValue(out) == false);
    CHECK(out == 7);  // unchanged when not cached
    CHECK_FALSE(cache == 0);  // operator== should be false when not cached
}

TEST_CASE("CachedValue: set and get with primitive type") {
    CachedValue<int> cache;

    cache.set(42);
    CHECK(cache.isCached());

    int out = 0;
    REQUIRE(cache.getValue(out));
    CHECK(out == 42);
    CHECK(cache == 42);
    CHECK_FALSE(cache == 7);

    cache.set(7);
    REQUIRE(cache.getValue(out));
    CHECK(out == 7);
}

TEST_CASE("CachedValue: reset clears cached state") {
    CachedValue<std::string> cache;
    cache.set("ready");
    REQUIRE(cache.isCached());

    cache.reset();
    CHECK(cache.isCached() == false);

    std::string out = "keep";
    CHECK(cache.getValue(out) == false);
    CHECK(out == "keep");
    CHECK_FALSE(cache == std::string("ready"));
}
