#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>
#include <sti/device/ImportedFile.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include "LocalPersistenceManager.h"
#include "fileholder_tests_support.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

using fileholder_test_support::TempDir;
using fileholder_test_support::makeFileID;

using STI::Device::DeviceID;
using STI::Device::ImportedFile;
using STI::Device::ImportFileOptions;
using STI::Device::ImportStorage;
using STI::Device::LocalPersistenceManager;
using STI::Utils::Configuration;
using STI::Utils::FileID;
using STI::Utils::FileServer;
using STI::Utils::FileTransferType;
using STI::Utils::LocalFileHolderFactory;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;

namespace {

DeviceID makeTargetDeviceID()
{
    return DeviceID("Target", "127.0.0.1", 2);
}

DeviceID makeSourceDeviceID()
{
    return DeviceID("Source", "127.0.0.1", 3);
}

std::shared_ptr<LocalPersistenceManager> makePersistenceManager(
    const DeviceID& deviceID,
    const std::filesystem::path& basePath,
    std::optional<unsigned> importMaxBytes = std::nullopt)
{
    Configuration config;
    if (importMaxBytes.has_value()) {
        config.set("PersistenceManager", "importMaxBytes", importMaxBytes.value());
    }

    auto holderFactory = std::make_shared<LocalFileHolderFactory>(deviceID.getID());
    return std::make_shared<LocalPersistenceManager>(
        deviceID,
        config,
        basePath.string(),
        holderFactory,
        nullptr,
        nullptr);
}

FileID makeSourceFileID(const std::string& filename = "payload.bin")
{
    return makeFileID(makeSourceDeviceID(), "source/path", filename);
}

std::shared_ptr<VirtualFileServer> makeSourceServer(const FileID& sourceID, const std::string& payload)
{
    auto sourceServer = std::make_shared<VirtualFileServer>();
    auto sourceHolder = std::make_shared<VirtualFileHolder>(sourceID.origin, sourceID);

    REQUIRE(sourceHolder->openFile());
    REQUIRE(sourceHolder->write(payload.data(), static_cast<unsigned>(payload.size())));
    sourceHolder->closeFile();
    REQUIRE(sourceServer->addFile(sourceHolder));

    return sourceServer;
}

std::shared_ptr<FileServer> getFileServer(const std::shared_ptr<LocalPersistenceManager>& manager)
{
    std::shared_ptr<FileServer> server;
    REQUIRE(manager->getFileServer(server));
    REQUIRE(server != nullptr);
    return server;
}

std::string readImportedBytes(
    const std::shared_ptr<LocalPersistenceManager>& manager,
    const FileID& importedID)
{
    auto destinationID = makeFileID(DeviceID("Reader", "127.0.0.1", 4), "reader", "copy.bin");
    auto destination = std::make_shared<VirtualFileHolder>(destinationID.origin, destinationID);
    REQUIRE(getFileServer(manager)->transferFile(importedID, destination, FileTransferType::Binary));
    return destination->getBytes();
}

} // namespace

TEST_CASE("LocalPersistenceManager: importFile defaults to disk temporary storage")
{
    TempDir temp("sti3-import-disk-");
    auto targetID = makeTargetDeviceID();
    auto manager = makePersistenceManager(targetID, temp.path);

    const std::string payload = "disk import payload";
    auto sourceID = makeSourceFileID("image.dat");
    auto sourceServer = makeSourceServer(sourceID, payload);

    auto imported = manager->importFile(sourceID, sourceServer);
    REQUIRE(imported != nullptr);

    auto importedID = imported->getFileID();
    CHECK(importedID.persistenceLocation == targetID.getID());
    CHECK(importedID.filename == "image.dat");
    CHECK(std::filesystem::path(importedID.path) == std::filesystem::path(manager->getTemporaryPath()) / "imports");
    CHECK(std::filesystem::exists(importedID.getFullFilename()));
    CHECK(readImportedBytes(manager, importedID) == payload);

    REQUIRE(imported->close());
    CHECK(imported->isClosed());
    CHECK_FALSE(std::filesystem::exists(importedID.getFullFilename()));
    CHECK_FALSE(getFileServer(manager)->findFile(importedID));
}

TEST_CASE("LocalPersistenceManager: unique imports allow repeated source FileIDs")
{
    TempDir temp("sti3-import-unique-");
    auto manager = makePersistenceManager(makeTargetDeviceID(), temp.path);

    const std::string payload = "same source id payload";
    auto sourceID = makeSourceFileID("transient.bin");
    auto sourceServer = makeSourceServer(sourceID, payload);

    auto first = manager->importFile(sourceID, sourceServer);
    auto second = manager->importFile(sourceID, sourceServer);

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    CHECK(first->getFileID() != second->getFileID());
    CHECK(readImportedBytes(manager, first->getFileID()) == payload);
    CHECK(readImportedBytes(manager, second->getFileID()) == payload);

    CHECK(first->close());
    CHECK(second->close());
}

TEST_CASE("LocalPersistenceManager: importMaxBytes rejects oversized imports")
{
    TempDir temp("sti3-import-size-");
    auto manager = makePersistenceManager(makeTargetDeviceID(), temp.path, 5);

    auto sourceID = makeSourceFileID("too-large.bin");
    auto sourceServer = makeSourceServer(sourceID, "123456");

    auto imported = manager->importFile(sourceID, sourceServer);

    CHECK(imported == nullptr);
}

TEST_CASE("LocalPersistenceManager: virtual imports are registered and released by handle")
{
    TempDir temp("sti3-import-virtual-");
    auto manager = makePersistenceManager(makeTargetDeviceID(), temp.path);

    const std::string payload = "virtual import payload";
    auto sourceID = makeSourceFileID("virtual.bin");
    auto sourceServer = makeSourceServer(sourceID, payload);

    ImportFileOptions options;
    options.storage = ImportStorage::Virtual;

    auto imported = manager->importFile(sourceID, sourceServer, options);
    REQUIRE(imported != nullptr);

    auto importedID = imported->getFileID();
    CHECK(getFileServer(manager)->findFile(importedID));
    CHECK(readImportedBytes(manager, importedID) == payload);

    REQUIRE(imported->close());
    CHECK(imported->isClosed());
    CHECK_FALSE(getFileServer(manager)->findFile(importedID));
}
