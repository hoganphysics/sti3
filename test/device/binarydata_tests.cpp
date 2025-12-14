#include <catch2/catch_test_macros.hpp>

#include <sti/utils/BinaryData.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

using STI::Utils::BinaryData;

namespace
{
struct CountingStruct
{
    static int destructCount;
    CountingStruct() = default;
    ~CountingStruct()
    {
        ++destructCount;
    }
};

int CountingStruct::destructCount = 0;
} // namespace

TEST_CASE("BinaryData stores typed buffers and exposes metadata")
{
    BinaryData data;
    auto* values = new int[3]{1, 2, 3};
    data.assign(values, 3);

    int* retrieved = nullptr;
    REQUIRE(data.get<int>(retrieved));
    CHECK(retrieved == values);
    CHECK(data.isType<int*>());
    CHECK_FALSE(data.isType<double*>());

    CHECK(data.length() == 3);
    CHECK(data.wordsize() == sizeof(int));
    CHECK(data.bytes() == 3 * sizeof(int));

    char* bytesPtr = nullptr;
    REQUIRE(data.getBytes(bytesPtr));
    CHECK(reinterpret_cast<void*>(bytesPtr) == static_cast<void*>(values));
}

TEST_CASE("BinaryData equality reflects underlying pointer and swap exchanges ownership")
{
    BinaryData first;
    auto* firstBuffer = new char[4];
    std::memcpy(firstBuffer, "abcd", 4);
    first.assign(firstBuffer, 4);

    BinaryData second;
    auto* secondBuffer = new char[2];
    std::memcpy(secondBuffer, "xy", 2);
    second.assign(secondBuffer, 2);

    BinaryData alias;
    alias.assign(firstBuffer, 4, false);

    CHECK(first == alias);
    CHECK(first != second);

    first.swap(second);

    char* swappedBytes = nullptr;
    REQUIRE(first.getBytes(swappedBytes));
    CHECK(std::string(swappedBytes, first.bytes()) == "xy");

    REQUIRE(second.getBytes(swappedBytes));
    CHECK(std::string(swappedBytes, second.bytes()) == "abcd");
}

TEST_CASE("BinaryData orphaned retrieval prevents automatic deletion")
{
    CountingStruct::destructCount = 0;

    BinaryData data;
    auto* owned = data.allocate<CountingStruct>(2);

    CountingStruct* retrieved = nullptr;
    REQUIRE(data.get<CountingStruct>(retrieved, true));
    CHECK(retrieved == owned);

    data.clear();
    CHECK(CountingStruct::destructCount == 0);

    delete[] retrieved;
    CHECK(CountingStruct::destructCount == 2);
}

TEST_CASE("BinaryData splits and merges byte buffers")
{
    const std::string text = "hello world";

    BinaryData data;
    auto* buffer = new char[text.size()];
    std::memcpy(buffer, text.data(), text.size());
    data.assign(buffer, text.size());

    std::vector<std::shared_ptr<BinaryData>> chunks;
    data.split(chunks, 4);

    REQUIRE(chunks.size() == 3);

    auto chunkString = [](const std::shared_ptr<BinaryData>& chunk) {
        char* bytes = nullptr;
        REQUIRE(chunk->getBytes(bytes));
        return std::string(bytes, chunk->bytes());
    };

    CHECK(chunkString(chunks[0]) == text.substr(0, 4));
    CHECK(chunkString(chunks[1]) == text.substr(4, 4));
    CHECK(chunkString(chunks[2]) == text.substr(8));

    BinaryData merged;
    merged.merge(chunks);

    char* mergedBytes = nullptr;
    REQUIRE(merged.getBytes(mergedBytes));
    CHECK(std::string(mergedBytes, merged.bytes()) == text);
}
