#include <catch2/catch_test_macros.hpp>

#include <sti/utils/VirtualFileServer.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/LocalFileHolder.h>

#include "fileholder_tests_support.h"

#include <memory>
#include <string>

using fileholder_test_support::TempDir;
using fileholder_test_support::makeFileID;
using fileholder_test_support::makeTestDeviceID;
using fileholder_test_support::readFileToString;

using STI::Utils::FileTransferType;
using STI::Utils::LocalFileHolder;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;

TEST_CASE("VirtualFileServer: add, find, size, and delete") {
    VirtualFileServer server;
    TempDir td;

    auto localHolder = std::make_shared<LocalFileHolder>("origin", td.path.string(), "file.txt");
    const std::string payload = "content";
    REQUIRE(localHolder->openFile());
    REQUIRE(localHolder->write(payload.data(), static_cast<unsigned>(payload.size())));
    localHolder->closeFile();
    REQUIRE(server.addFile(localHolder));
    auto fid = localHolder->getID();

    CHECK(server.findFile(fid));
    CHECK(server.getFileSize(fid) == 7);
    CHECK(server.deleteFile(fid));
    CHECK_FALSE(server.findFile(fid));
}

TEST_CASE("VirtualFileServer: transfer stored virtual file to local file") {
    VirtualFileServer server;
    auto device = makeTestDeviceID();
    auto fid = makeFileID(device, "virtual/path", "payload.bin");

    auto source = std::make_shared<VirtualFileHolder>(device.getID(), fid);
    source->openFile();
    (*source) << "payload";
    REQUIRE(server.addFile(source));

    TempDir td;
    auto destination = std::make_shared<LocalFileHolder>("dest", td.path.string(), "out.bin");

    REQUIRE(server.transferFile(fid, destination, FileTransferType::Binary));
    CHECK(readFileToString(destination->getFilename()) == "payload");
}
