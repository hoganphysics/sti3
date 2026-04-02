#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalFileServer.h"
#include <sti/utils/LocalFileHolder.h>

#include "fileholder_tests_support.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

using fileholder_test_support::TempDir;
using fileholder_test_support::makeFileID;
using fileholder_test_support::makeTestDeviceID;
using fileholder_test_support::readFileToString;

using STI::Device::DeviceID;
using STI::Utils::FileID;
using STI::Utils::LocalFileHolder;
using STI::Utils::LocalFileServer;

TEST_CASE("LocalFileServer: find and size honor persistence location") {
    TempDir td;
    DeviceID localDevice = makeTestDeviceID();
    LocalFileServer server(localDevice);

    auto filePath = td.path / "data.bin";
    {
        std::ofstream ofs(filePath, std::ios::binary);
        ofs << "abc";
    }

    FileID fid = makeFileID(localDevice, td.path, "data.bin");

    CHECK(server.findFile(fid));
    CHECK(server.getFileSize(fid) == 3);

    FileID remoteFID = fid;
    remoteFID.persistenceLocation = "remote";
    CHECK_FALSE(server.findFile(remoteFID));
    CHECK(server.getFileSize(remoteFID) == false); // returns 0/false when not found
}

TEST_CASE("LocalFileServer: transfer and delete") {
    TempDir td;
    DeviceID localDevice = makeTestDeviceID();
    LocalFileServer server(localDevice);

    auto sourcePath = td.path / "src";
    auto destPath = td.path / "dest";
    std::filesystem::create_directories(sourcePath);

    auto sourceFile = sourcePath / "payload.txt";
    const std::string payload = "server-transfer";
    {
        std::ofstream ofs(sourceFile, std::ios::binary);
        ofs << payload;
    }

    FileID sourceID = makeFileID(localDevice, sourcePath, "payload.txt");
    auto destination = std::make_shared<LocalFileHolder>("dest-origin", destPath.string(), "copy.txt");

    REQUIRE(server.transferFile(sourceID, destination, STI::Utils::FileTransferType::Binary));
    CHECK(destination->exists());
    CHECK(readFileToString(destination->getFilename()) == payload);

    CHECK(server.deleteFile(sourceID));
    CHECK_FALSE(server.findFile(sourceID));
    CHECK_FALSE(server.deleteFile(sourceID)); // already gone
}

TEST_CASE("LocalFileServer: transferFilePartial returns line windows") {
    TempDir td;
    DeviceID localDevice = makeTestDeviceID();
    LocalFileServer server(localDevice);

    auto sourcePath = td.path / "src";
    auto destPath = td.path / "dest";
    std::filesystem::create_directories(sourcePath);

    auto sourceFile = sourcePath / "payload.txt";
    {
        std::ofstream ofs(sourceFile);
        ofs << "line-0\nline-1\nline-2\nline-3\nline-4\n";
    }

    FileID sourceID = makeFileID(localDevice, sourcePath, "payload.txt");

    SECTION("positive offsets count from the beginning") {
        auto destination = std::make_shared<LocalFileHolder>("dest-origin", destPath.string(), "slice-positive.txt");

        REQUIRE(server.transferFilePartial(sourceID, destination, 1, 2));
        CHECK(readFileToString(destination->getFilename()) == "line-1\nline-2\n");
    }

    SECTION("negative offsets count backward from the end") {
        auto destination = std::make_shared<LocalFileHolder>("dest-origin", destPath.string(), "slice-negative.txt");

        REQUIRE(server.transferFilePartial(sourceID, destination, -3, 2));
        CHECK(readFileToString(destination->getFilename()) == "line-2\nline-3\n");
    }
}
