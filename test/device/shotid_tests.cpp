#include <catch2/catch_test_macros.hpp>

#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ShotID.h>
#include <sti/utils/TimeStamp.h>

using STI::Engine::EngineJobSourceID;
using STI::Engine::ParseID;
using STI::Engine::ShotID;
using STI::Utils::TimeStamp;

TEST_CASE("ShotID print includes full submission date and time", "[shotid] [id]")
{
    EngineJobSourceID source("user", "machine");
    ParseID parseID(TimeStamp(2026, 5, 2, 15, 3, 5, 713, 998, 140), source);
    TimeStamp submissionTime(2026, 5, 2, 17, 37, 24, 137, 172, 644);

    ShotID shotID(parseID, source, submissionTime);

    CHECK(shotID.print() == "sid:user@machine#2026/05/02|17:37:24.137.172.644");
}
