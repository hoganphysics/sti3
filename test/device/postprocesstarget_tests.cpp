#include <catch2/catch_test_macros.hpp>

#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/RawEventTargetDevice.h>
#include <sti/device/DeviceID.h>

#include <sti/extern/cereal/archives/xml.hpp>

#include <sstream>
#include <string>

using STI::Engine::PostProcessTarget;
using STI::Engine::RawEventTargetDevice;
using STI::Device::DeviceID;


TEST_CASE("PostProcessTarget: name-only construction is abstract", "[postprocessing]") {
    PostProcessTarget target("Analysis", "Blue MOT");

    CHECK(target.isAbstract());
    CHECK(target.name() == "Blue MOT");
    CHECK(target.device().name() == "Analysis");
    CHECK(target.device().isAbstract());
}

TEST_CASE("PostProcessTarget: construction from a resolved device is concrete", "[postprocessing]") {
    DeviceID id("Analysis", "localhost", 0);
    RawEventTargetDevice resolved(id);
    REQUIRE_FALSE(resolved.isAbstract());

    PostProcessTarget target(resolved, "Blue MOT");

    CHECK_FALSE(target.isAbstract());
    CHECK(target.name() == "Blue MOT");
    CHECK(target.device().deviceID() == id);
}

TEST_CASE("PostProcessTarget: equality and ordering", "[postprocessing]") {
    PostProcessTarget a("Analysis", "Blue MOT");
    PostProcessTarget b("Analysis", "Blue MOT");
    PostProcessTarget c("Analysis", "Red MOT");
    PostProcessTarget d("Other", "Blue MOT");

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(a != c);
    CHECK(a != d);

    // strict weak ordering: exactly one of a<c, c<a holds for distinct targets
    CHECK((a < c) != (c < a));
    CHECK_FALSE(a < b);
    CHECK_FALSE(b < a);
}

TEST_CASE("PostProcessTarget: cereal XML round-trip preserves fields", "[postprocessing]") {
    DeviceID id("Analysis", "localhost", 2);
    PostProcessTarget original(RawEventTargetDevice(id), "Blue MOT");

    std::stringstream stream;
    {
        cereal::XMLOutputArchive out(stream);
        out(original);
    }

    PostProcessTarget restored;
    {
        cereal::XMLInputArchive in(stream);
        in(restored);
    }

    CHECK(restored.name() == "Blue MOT");
    CHECK(restored.isAbstract() == original.isAbstract());
    CHECK(restored.device().deviceID() == id);
    CHECK(restored == original);
}
