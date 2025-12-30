#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <string>
#include <vector>

using STI::Utils::MetaData;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

TEST_CASE("MetaData: add, contains, and get") {
    MetaData meta;
    MixedValue val;
    val.setValue(42);

    CHECK_FALSE(meta.contains("answer"));
    meta.addMetaData("answer", val);
    CHECK(meta.contains("answer"));

    MixedValue out = meta.getMetaData("answer");
    CHECK(out.getInt() == 42);

    auto keys = meta.keys();
    REQUIRE(keys.size() == 1);
    CHECK(keys[0] == "answer");
}

TEST_CASE("MetaData: reset and remove entries") {
    MetaData meta;
    MixedValue first;
    first.setValue(std::string("first"));
    MixedValue second;
    second.setValue(std::string("second"));

    meta.addMetaData("key", first);
    REQUIRE(meta.contains("key"));
    CHECK(meta.getMetaData("key").getString() == "first");

    REQUIRE(meta.resetMetaDataEntry("key", second));
    CHECK(meta.getMetaData("key").getString() == "second");

    meta.removeMetaData("key");
    CHECK_FALSE(meta.contains("key"));
    CHECK(meta.getMetaData("key").isEmpty());
    CHECK(meta.keys().empty());
}

TEST_CASE("MetaData: construct from MixedValue vector and merge") {
    // Build a MixedValue vector of tuples [ [k,v], [k2,v2] ]
    MixedValue tuple1;
    tuple1.addValue("k1");
    tuple1.addValue(1);
    MixedValue tuple2;
    tuple2.addValue("k2");
    tuple2.addValue(std::string("v2"));

    MixedValue vec;
    vec.addValue(tuple1);
    vec.addValue(tuple2);

    MetaData meta(vec);
    CHECK(meta.contains("k1"));
    CHECK(meta.contains("k2"));
    CHECK(meta.getMetaData("k1").getInt() == 1);
    CHECK(meta.getMetaData("k2").getString() == "v2");

    MetaData other;
    MixedValue newVal;
    newVal.setValue(3.14);
    other.addMetaData("k3", newVal);

    meta.merge(other);
    CHECK(meta.contains("k3"));
    CHECK(meta.getMetaData("k3").getDouble() == Catch::Approx(3.14));
    auto keys = meta.keys();
    REQUIRE(keys.size() == 3);
}

TEST_CASE("MetaData: merge overwrites existing keys and keeps others") {
    MetaData original;
    MixedValue one;
    one.setValue(1);
    original.addMetaData("k1", one);

    MetaData incoming;
    MixedValue newOne;
    newOne.setValue(2);
    incoming.addMetaData("k1", newOne);  // should overwrite
    MixedValue extra;
    extra.setValue(std::string("new"));
    incoming.addMetaData("k2", extra);   // should be added

    original.merge(incoming);

    CHECK(original.getMetaData("k1").getInt() == 2);
    CHECK(original.getMetaData("k2").getString() == "new");

    auto keys = original.keys();
    REQUIRE(keys.size() == 2);
    CHECK((keys == std::vector<std::string>{"k1", "k2"} || keys == std::vector<std::string>{"k2", "k1"}));
}

TEST_CASE("MetaData: clear empties data") {
    MetaData meta;
    MixedValue val;
    val.setValue(5);
    meta.addMetaData("a", val);
    REQUIRE(meta.contains("a"));

    meta.clear();
    CHECK_FALSE(meta.contains("a"));
    CHECK(meta.keys().empty());
    CHECK(meta.getMetaData().getVector().empty());
}
