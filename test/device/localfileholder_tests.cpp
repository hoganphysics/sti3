#include <catch2/catch_test_macros.hpp>

#include <sti/utils/LocalFileHolder.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <chrono>

using STI::Utils::FileHolder;
using STI::Utils::LocalFileHolder;

namespace {

// Simple RAII temp directory for file I/O tests.
struct TempDir {
    std::filesystem::path path;

    TempDir(std::string prefix = "sti3-localfileholder-") {
        auto base = std::filesystem::temp_directory_path();
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937_64 rng(static_cast<uint64_t>(now));
        auto suffix = rng();
        path = base / (prefix + std::to_string(now) + "-" + std::to_string(suffix));
        std::filesystem::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

constexpr const char* kOriginA = "origin-A";
constexpr const char* kOriginB = "origin-B";

std::string readFileToString(const std::filesystem::path& file) {
    std::ifstream ifs(file, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("LocalFileHolder: construction, equality, and filename") {
    TempDir td;
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
    TempDir td;
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
    TempDir td;
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
    CHECK(readFileToString(destination->getFilename()) == payload);
    // Source metadata adopts destination origin and creationTime.
    CHECK(source.getID().origin == kOriginB);
    CHECK(source.getID().creationTime == destination->getID().creationTime);
}

TEST_CASE("LocalFileHolder: transferFile failure cases") {
    TempDir td;
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
    TempDir td;
    LocalFileHolder holder(kOriginA, td.path.string(), "file.bin");
    CHECK(holder.maxBufferSize() == 16384);
}
