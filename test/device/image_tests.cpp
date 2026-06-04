#include <catch2/catch_test_macros.hpp>

#include <sti/utils/Image.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/BinaryDataStream.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/LocalFileHolder.h>

#include "fileholder_tests_support.h"

#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

using fileholder_test_support::TempDir;
using fileholder_test_support::readFileToString;

using STI::Utils::BinaryData;
using STI::Utils::BinaryDataStream;
using STI::Utils::BinaryDataStreamTarget;
using STI::Utils::FileHolder;
using STI::Utils::FileServer;
using STI::Utils::FileTransferType;
using STI::Utils::Image;
using STI::Utils::LocalFileHolder;

namespace {

class RecordingFileServer : public FileServer {
public:
    bool findFile(const STI::Utils::FileID&) override { return false; }
    int getFileSize(const STI::Utils::FileID&) override { return 0; }
    bool transferFile(const STI::Utils::FileID& source, const std::shared_ptr<FileHolder>& destination, FileTransferType type) override {
        ++transferCount;
        lastSource = source;
        lastDestination = destination;
        lastType = type;
        return transferResult;
    }
    bool transferFilePartial(const STI::Utils::FileID&, const std::shared_ptr<FileHolder>&, int, int) override { return false; }
    bool deleteFile(const STI::Utils::FileID&) override { return false; }

    unsigned transferCount{0};
    STI::Utils::FileID lastSource;
    std::shared_ptr<FileHolder> lastDestination;
    FileTransferType lastType{FileTransferType::Binary};
    bool transferResult{true};
};

class PayloadStream : public BinaryDataStream {
public:
    explicit PayloadStream(std::string payload) : payload(std::move(payload)) {}

    void transfer(const std::shared_ptr<BinaryDataStreamTarget>& target) override {
        ++transferCount;
        if (target == nullptr) {
            return;
        }

        auto chunk = std::make_shared<BinaryData>();
        auto* buffer = new char[payload.size()];
        std::memcpy(buffer, payload.data(), payload.size());
        chunk->assign(buffer, payload.size());

        target->start();
        target->writeNext(chunk);
        target->stop();
    }

    std::string payload;
    unsigned transferCount{0};
};

} // namespace

TEST_CASE("Image: constructor seeds file ID") {
    TempDir td;
    auto filename = td.path / "frames" / "frame01.tif";

    Image image("origin-device", filename.string());
    auto id = image.getFileID();

    CHECK(id.origin == "origin-device");
    CHECK(id.persistenceLocation == "origin-device");
    CHECK(id.filename == "frame01.tif");
    CHECK(std::filesystem::path(id.path) == filename.parent_path());
}

TEST_CASE("Image: height and width setters populate metadata") {
    Image image;
    image.setHeight(480).setWidth(640);

    CHECK(image.getHeight() == 480);
    CHECK(image.getWidth() == 640);
    CHECK(image.metaData.contains("height"));
    CHECK(image.metaData.contains("width"));
    CHECK(image.metaData.getMetaData("height").getInt() == 480);
    CHECK(image.metaData.getMetaData("width").getInt() == 640);
}

TEST_CASE("Image: BinaryData setters cache data and retrieval succeeds") {
    Image image;
    const std::string payload = "binary-image-data";

    auto* buffer = new char[payload.size()];
    std::memcpy(buffer, payload.data(), payload.size());
    image.setImageData(buffer, payload.size());

    std::shared_ptr<BinaryData> data;
    REQUIRE(image.getData(data));
    char* bytes = nullptr;
    REQUIRE(data->getBytes(bytes));
    CHECK(std::string(bytes, data->bytes()) == payload);

    std::shared_ptr<FileHolder> file;
    CHECK_FALSE(image.getFile(file));
}

TEST_CASE("Image: saveToFile returns false when only BinaryData is set") {
    TempDir td;
    auto path = td.path / "captures" / "capture.bin";
    const std::string payload = "image-payload";

    Image image("local-origin", path.string());
    auto data = std::make_shared<BinaryData>();
    auto* buffer = new char[payload.size()];
    std::memcpy(buffer, payload.data(), payload.size());
    data->assign(buffer, payload.size());
    image.setImageData(data);

    CHECK_FALSE(image.saveToFile());
    CHECK_FALSE(std::filesystem::exists(path));

    std::shared_ptr<FileHolder> holder;
    CHECK_FALSE(image.getFile(holder));
}

TEST_CASE("Image: write writes BinaryData into destination FileHolder and updates fileID") {
    TempDir td;
    auto destPath = td.path / "out";
    const std::string payload = "write-binary-data";

    Image image("source-origin", (td.path / "source" / "ignored.bin").string());
    auto data = std::make_shared<BinaryData>();
    auto* buffer = new char[payload.size()];
    std::memcpy(buffer, payload.data(), payload.size());
    data->assign(buffer, payload.size());
    image.setImageData(data);

    auto destination = std::make_shared<LocalFileHolder>("dest-origin", destPath.string(), "image.bin");
    auto fileServer = std::make_shared<RecordingFileServer>();

    REQUIRE(image.write(fileServer, destination));

    CHECK(readFileToString(destination->getFilename()) == payload);
    auto id = image.getFileID();
    CHECK(id.filename == "image.bin");
    CHECK(id.origin == "source-origin");
    CHECK(std::filesystem::path(id.path) == destPath);
    CHECK(fileServer->transferCount == 0); // BinaryData path bypasses transferFile

    std::shared_ptr<FileHolder> cached;
    REQUIRE(image.getFile(cached));
    CHECK(cached == destination);
}

TEST_CASE("Image: write pulls lazy BinaryData and clears cached stream data") {
    TempDir td;
    auto destPath = td.path / "out";
    const std::string payload = "lazy-image-payload";

    auto stream = std::make_shared<PayloadStream>(payload);
    auto data = std::make_shared<BinaryData>();
    data->attachStream(stream, payload.size(), 1);

    Image image("source-origin", (td.path / "source" / "ignored.bin").string());
    image.setImageData(data);

    auto destination = std::make_shared<LocalFileHolder>("dest-origin", destPath.string(), "image.bin");
    auto fileServer = std::make_shared<RecordingFileServer>();

    REQUIRE(image.write(fileServer, destination));

    CHECK(stream->transferCount == 1);
    CHECK(readFileToString(destination->getFilename()) == payload);

    std::shared_ptr<BinaryData> cachedData;
    CHECK_FALSE(image.getData(cachedData));

    std::shared_ptr<FileHolder> cachedFile;
    REQUIRE(image.getFile(cachedFile));
    CHECK(cachedFile == destination);
}

TEST_CASE("Image: write delegates to FileServer when FileHolder is cached") {
    TempDir td;
    auto sourcePath = td.path / "source";
    auto destinationPath = td.path / "destination";

    auto holder = std::make_shared<LocalFileHolder>("holder-origin", sourcePath.string(), "image.dat");
    Image image;
    image.setImageData(holder);
    image.setFilename("image.dat");

    auto destination = std::make_shared<LocalFileHolder>("dest-origin", destinationPath.string(), "copy.dat");
    auto fileServer = std::make_shared<RecordingFileServer>();

    REQUIRE(image.write(fileServer, destination));
    CHECK(fileServer->transferCount == 1);
    CHECK(fileServer->lastDestination == destination);
    CHECK(fileServer->lastSource == holder->getID());
    CHECK(fileServer->lastType == FileTransferType::Binary);
}

TEST_CASE("Image: copy constructor preserves cached data and metadata") {
    TempDir td;
    auto holder = std::make_shared<LocalFileHolder>("holder-origin", td.path.string(), "image.raw");

    Image original;
    original.setHeight(100).setWidth(200);
    auto data = std::make_shared<BinaryData>();
    auto* buffer = new char[3]{'x', 'y', 'z'};
    data->assign(buffer, 3);
    original.setImageData(data);
    original.setImageData(holder);

    Image copy(original);

    std::shared_ptr<BinaryData> copiedData;
    REQUIRE(copy.getData(copiedData));
    char* copiedBytes = nullptr;
    REQUIRE(copiedData->getBytes(copiedBytes));
    CHECK(std::string(copiedBytes, copiedData->bytes()) == "xyz");

    std::shared_ptr<FileHolder> copiedHolder;
    REQUIRE(copy.getFile(copiedHolder));
    CHECK(copiedHolder == holder);

    CHECK(copy.getHeight() == 100);
    CHECK(copy.getWidth() == 200);
    CHECK(copy.metaData.contains("height"));
    CHECK(copy.metaData.contains("width"));
}

TEST_CASE("Image: equality compares FileIDs only") {
    STI::Utils::FileID fileID;
    fileID.origin = "origin";
    fileID.persistenceLocation = "origin";
    fileID.path = "/data/images";
    fileID.filename = "frame.raw";

    Image first(fileID);
    Image second(fileID);

    Image different(fileID);
    different.setFilename("other.raw");

    CHECK(first == second);
    CHECK(first != different);
}
