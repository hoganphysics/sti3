#include <catch2/catch_test_macros.hpp>

#include <sti/utils/LocalFileHolder.h>

#include "fileholder_tests_support.h"

#include <filesystem>
#include <memory>
#include <string>

using STI::Utils::FileHolder;
using STI::Utils::LocalFileHolder;

namespace {

constexpr const char* kOriginA = "origin-A";
constexpr const char* kOriginB = "origin-B";

} // namespace

TEST_CASE("LocalFileHolder: construction, equality, and filename") {
    fileholder_test_support::TempDir td;
    LocalFileHolder holderA(kOriginA, td.path.string(), "file.bin");
    LocalFileHolder holderB(kOriginA, td.path.string(), "file.bin");
    LocalFileHolder holderC(kOriginA, td.path.string(), "other.bin");

    CHECK(holderA.getID().origin == kOriginA);
    CHECK(holderA.getID().persistenceLocation == kOriginA);
    CHECK(holderA.getFilename() == (td.path / "file.bin").string());

    CHECK(holderA == holderB);
    CHECK(holderA != holderC);
}

TEST_CASE("LocalFileHolder: open, write, exists, size, and md5") {
    fileholder_test_support::TempDir td;
    LocalFileHolder holder(kOriginA, td.path.string(), "payload.dat");
    const std::string payload = "hello world\n";

    REQUIRE(holder.openFile());
    REQUIRE(holder.write(payload.data(), static_cast<unsigned>(payload.size())));
    holder.closeFile();

    REQUIRE(holder.exists());
    CHECK(holder.getFileSize() == static_cast<unsigned>(payload.size()));

    const std::string expectedMd5 = "6F5902AC237024BDD0C176CB93063DC4"; // md5 of "hello world\n"
    CHECK(holder.md5Checksum() == expectedMd5);
    // Cached value should match on repeat call.
    CHECK(holder.md5Checksum() == expectedMd5);
}

TEST_CASE("LocalFileHolder: transferFile copies content and updates metadata") {
    fileholder_test_support::TempDir td;
    auto sourcePath = td.path / "src";
    auto destPath = td.path / "dest";
    LocalFileHolder source(kOriginA, sourcePath.string(), "source.bin");
    auto destination = std::make_shared<LocalFileHolder>(kOriginB, destPath.string(), "dest.bin");

    const std::string payload = "transfer-data-123";
    REQUIRE(source.openFile());
    REQUIRE(source.write(payload.data(), static_cast<unsigned>(payload.size())));
    source.closeFile();

    REQUIRE_FALSE(destination->exists());
    REQUIRE(source.transferFile(destination));

    CHECK(destination->exists());
    CHECK(fileholder_test_support::readFileToString(destination->getFilename()) == payload);
    // Source metadata adopts destination origin and creationTime.
    CHECK(source.getID().origin == kOriginB);
    CHECK(source.getID().creationTime == destination->getID().creationTime);
}

TEST_CASE("LocalFileHolder: transferFile failure cases") {
    fileholder_test_support::TempDir td;
    auto sourcePath = td.path / "src";
    auto destPath = td.path / "dest";
    LocalFileHolder source(kOriginA, sourcePath.string(), "source.bin");

    const std::string payload = "abc";
    REQUIRE(source.openFile());
    REQUIRE(source.write(payload.data(), static_cast<unsigned>(payload.size())));
    source.closeFile();

    SECTION("destination already exists") {
        auto destination = std::make_shared<LocalFileHolder>(kOriginB, destPath.string(), "dest.bin");
        REQUIRE(destination->openFile());
        REQUIRE(destination->write(payload.data(), static_cast<unsigned>(payload.size())));
        destination->closeFile();
        REQUIRE(destination->exists());

        CHECK_FALSE(source.transferFile(destination));
    }

    SECTION("null destination") {
        std::shared_ptr<FileHolder> nullDest;
        CHECK_FALSE(source.transferFile(nullDest));
    }
}

TEST_CASE("LocalFileHolder: maxBufferSize constant") {
    fileholder_test_support::TempDir td;
    LocalFileHolder holder(kOriginA, td.path.string(), "file.bin");
    CHECK(holder.maxBufferSize() == 16384);
}
