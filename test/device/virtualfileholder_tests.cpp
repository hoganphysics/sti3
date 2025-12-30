#include <catch2/catch_test_macros.hpp>

#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/LocalFileHolder.h>

#include "fileholder_tests_support.h"

#include <memory>
#include <string>

using fileholder_test_support::TempDir;
using fileholder_test_support::makeFileID;
using fileholder_test_support::makeTestDeviceID;
using fileholder_test_support::readFileToString;

using STI::Utils::LocalFileHolder;
using STI::Utils::VirtualFileHolder;

TEST_CASE("VirtualFileHolder: open resets buffer and operator<< stores data") {
    auto device = makeTestDeviceID();
    auto fid = makeFileID(device, "virtual/path", "virtual.txt");
    VirtualFileHolder holder(device.getID(), fid);

    holder << "first";
    holder.openFile(); // resets internal buffer
    holder << "second";

    TempDir td;
    auto destination = std::make_shared<LocalFileHolder>("dest", td.path.string(), "out.txt");

    REQUIRE(holder.transferFile(destination));
    CHECK(readFileToString(destination->getFilename()) == "second");
}

TEST_CASE("VirtualFileHolder: transfer computes checksum and updates metadata") {
    auto device = makeTestDeviceID();
    auto fid = makeFileID(device, "virtual/path", "data.bin");
    VirtualFileHolder source(device.getID(), fid);

    source.openFile();
    source << "virtual-data";

    TempDir td;
    auto destination = std::make_shared<LocalFileHolder>("dest-origin", td.path.string(), "copied.bin");

    REQUIRE(source.transferFile(destination));
    CHECK(destination->exists());
    CHECK(source.getID().origin == "dest-origin");
    CHECK(readFileToString(destination->getFilename()) == "virtual-data");
}
