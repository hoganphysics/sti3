#include <catch2/catch_test_macros.hpp>

#include <sti/utils/FileID.h>

#include <filesystem>
#include <string>
#include <vector>

using STI::Utils::FileID;

TEST_CASE("FileID: equality, ordering, and paths") {
    FileID fidA;
    fidA.origin = "origin1";
    fidA.persistenceLocation = "persistA";
    fidA.path = "/data";
    fidA.filename = "file.txt";

    FileID fidB = fidA;
    CHECK(fidA == fidB);
    CHECK_FALSE(fidA != fidB);
    CHECK(fidA.getFullFilename() == "/data/file.txt");
    CHECK(fidA.print() == "<origin=origin1, file=/data/file.txt>");

    FileID fidC = fidA;
    fidC.filename = "z.txt";
    CHECK(fidA < fidC);
}

TEST_CASE("FileID: commonBasePath") {
    FileID f1;
    f1.path = "/data/project/run1";
    f1.filename = "a.bin";
    FileID f2;
    f2.path = "/data/project/run1/sub";
    f2.filename = "b.bin";

    std::vector<FileID> files{f1, f2};
    auto base = FileID::commonBasePath(files);
    CHECK(base == std::filesystem::path("/data/project/run1"));
}
