#include <catch2/catch_test_macros.hpp>

#include <sti/utils/BinaryData.h>
#include "LocalBinaryDataStream.h"

#include <cstring>
#include <memory>
#include <string>

using STI::Utils::BinaryData;
using STI::Utils::LocalBinaryDataStream;
using STI::Utils::LocalBinaryDataStreamTarget;

TEST_CASE("LocalBinaryDataStream transfers chunks into target BinaryData")
{
    const std::string payload = "abcdefghijkl";

    BinaryData source;
    auto* buffer = new char[payload.size()];
    std::memcpy(buffer, payload.data(), payload.size());
    source.assign(buffer, payload.size());

    auto target = std::make_shared<BinaryData>();

    LocalBinaryDataStream stream(&source, 3); // force multiple chunks
    auto targetStream = std::make_shared<LocalBinaryDataStreamTarget>(target);

    stream.transfer(targetStream);

    char* targetBytes = nullptr;
    REQUIRE(target->getBytes(targetBytes));
    CHECK(std::string(targetBytes, target->bytes()) == payload);

    char* sourceBytes = nullptr;
    REQUIRE(source.getBytes(sourceBytes));
    CHECK(targetBytes != sourceBytes); // merged data is a deep copy
}

TEST_CASE("LocalBinaryDataStream is a no-op when data or target is null")
{
    BinaryData source;
    auto target = std::make_shared<BinaryData>();

    LocalBinaryDataStream missingSource(nullptr, 4);
    missingSource.transfer(std::make_shared<LocalBinaryDataStreamTarget>(target));
    CHECK(target->length() == 0);

    auto* buffer = new char[2]{'x', 'y'};
    source.assign(buffer, 2);
    LocalBinaryDataStream stream(&source, 4);

    // Null target should not crash or modify the source
    stream.transfer(nullptr);

    char* sourceBytes = nullptr;
    REQUIRE(source.getBytes(sourceBytes));
    CHECK(std::string(sourceBytes, source.bytes()) == "xy");
}
