#include <catch2/catch_test_macros.hpp>

#include <sti/utils/VectorMap.h>

#include <string>
#include <vector>

using STI::Utils::VectorMap;

TEST_CASE("VectorMap: add, get, and indices") {
    VectorMap<std::string, int> map;

    unsigned idxA = map.add("a", 1);
    unsigned idxB = map.add("b", 2);
    CHECK(idxA == 0);
    CHECK(idxB == 1);
    CHECK(map.exists("a"));
    CHECK(map.exists("b"));
    CHECK(map.getVec().size() == 2);

    int value = 0;
    REQUIRE(map.get("b", value));
    CHECK(value == 2);

    unsigned idx = 99;
    REQUIRE(map.getIndex("a", idx));
    CHECK(idx == 0);
}

TEST_CASE("VectorMap: prepend, replace, and rename") {
    VectorMap<std::string, std::string> map;
    map.add("b", "two");
    map.add("c", "three");
    map.prepend("a", "one");  // shifts existing indices

    unsigned idx = 0;
    REQUIRE(map.getIndex("a", idx));
    CHECK(idx == 0);
    REQUIRE(map.getIndex("b", idx));
    CHECK(idx == 1);

    map.replace("b", "two-updated");
    std::string val;
    REQUIRE(map.get("b", val));
    CHECK(val == "two-updated");

    map.rename("c", "gamma");
    CHECK_FALSE(map.exists("c"));
    CHECK(map.exists("gamma"));
}

TEST_CASE("VectorMap: merge and clear") {
    VectorMap<std::string, int> left;
    left.add("l1", 1);
    left.add("l2", 2);

    VectorMap<std::string, int> right;
    right.add("r1", 10);
    right.add("r2", 20);

    REQUIRE(left.merge(right));
    CHECK(left.getVec().size() == 4);

    int val = 0;
    REQUIRE(left.get("r2", val));
    CHECK(val == 20);

    // overlapping key should prevent merge
    VectorMap<std::string, int> overlap;
    overlap.add("l1", 99);
    CHECK_FALSE(left.merge(overlap));

    left.clear();
    CHECK(left.getVec().empty());
}
