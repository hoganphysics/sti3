#include "FileTransferDevice.h"

#include <sti/device/PersistenceManager.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/Image.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

using STI::Utils::BinaryData;
using STI::Utils::FileHolder;
using STI::Utils::FileID;
using STI::Utils::FileServer;
using STI::Utils::Image;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace
{
void appendUInt16(std::vector<unsigned char>& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<unsigned char>(value & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
}

void appendUInt32(std::vector<unsigned char>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<unsigned char>(value & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
}

std::vector<unsigned char> ifdEntry(std::uint16_t tag,
                                    std::uint16_t fieldType,
                                    std::uint32_t count,
                                    std::uint32_t value,
                                    std::uint32_t value2 = 0)
{
    std::vector<unsigned char> entry;
    appendUInt16(entry, tag);
    appendUInt16(entry, fieldType);
    appendUInt32(entry, count);

    if (fieldType == 3 && count == 1) {
        appendUInt16(entry, static_cast<std::uint16_t>(value));
        appendUInt16(entry, 0);
    }
    else if (fieldType == 3 && count == 2) {
        appendUInt16(entry, static_cast<std::uint16_t>(value));
        appendUInt16(entry, static_cast<std::uint16_t>(value2));
    }
    else {
        appendUInt32(entry, value);
    }

    return entry;
}

void appendBytes(std::vector<unsigned char>& target, const std::vector<unsigned char>& source)
{
    target.insert(target.end(), source.begin(), source.end());
}

std::shared_ptr<BinaryData> makeBinaryData(const std::vector<unsigned char>& bytes)
{
    auto binaryData = std::make_shared<BinaryData>();
    auto* data = binaryData->allocate<unsigned char>(bytes.size());
    std::copy(bytes.begin(), bytes.end(), data);
    return binaryData;
}

bool writeBytes(const std::shared_ptr<FileHolder>& file, const std::vector<unsigned char>& bytes)
{
    if (file == nullptr || !file->openFile()) {
        return false;
    }

    bool success = bytes.empty()
        || file->write(reinterpret_cast<const char*>(bytes.data()), static_cast<unsigned>(bytes.size()));
    file->closeFile();
    return success;
}

bool writeText(const std::shared_ptr<FileHolder>& file, const std::string& text)
{
    if (file == nullptr || !file->openFile()) {
        return false;
    }

    bool success = text.empty()
        || file->write(text.data(), static_cast<unsigned>(text.size()));
    file->closeFile();
    return success;
}
}

FileTransferDevice::FileTransferDevice(const STI::Utils::Configuration& config)
    : STI::Device::LocalDevice(config)
{
    addOutputChannel(6, MixedValueType::File, "uploaded file (FileID)");

    addInputChannel(13, MixedValueType::Image, "random image (BinaryData)");
    addInputChannel(14, MixedValueType::Image, "random raw image (FileHolder)");
    addInputChannel(15, MixedValueType::Image, "random multi-pane TIF image (FileHolder)");
    addInputChannel(16, MixedValueType::File, "example text file (FileHolder)");
    addInputChannel(17, MixedValueType::Binary, "example binary data");
    addInputChannel(18, MixedValueType::File, "example virtual text file (VirtualFileHolder)");

    addInputChannel(19, MixedValueType::Image, MixedValueType::Image, "inverted image");
    addInputChannel(20, MixedValueType::Int, MixedValueType::File, "uploaded file size");

    addInputChannel(21, MixedValueType::Vector, "image vector measurement")
        .setVectorFormat({MixedValueType::Image, MixedValueType::String, MixedValueType::Double});
}

std::vector<unsigned char> FileTransferDevice::randomImageBytes(unsigned width,
                                                                unsigned height,
                                                                unsigned bytesPerPixel)
{
    std::vector<unsigned char> bytes(width * height * bytesPerPixel);

    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> byteDistribution(0, 255);

    std::generate(bytes.begin(), bytes.end(), [&byteDistribution]() {
        return static_cast<unsigned char>(byteDistribution(generator));
    });

    return bytes;
}

std::vector<unsigned char> FileTransferDevice::randomTifImageBytes()
{
    std::vector<std::vector<unsigned char>> panes;
    for (unsigned pane = 0; pane < tifPaneCount; ++pane) {
        panes.push_back(randomImageBytes(tifImageWidth, tifImageHeight, imageBytesPerPixel));
    }

    constexpr std::uint16_t ifdEntryCount = 12;
    constexpr std::uint32_t ifdOffset = 8;
    constexpr std::uint32_t ifdSize = 2 + (ifdEntryCount * 12) + 4;
    const std::uint32_t imageOffset = ifdOffset + (ifdSize * static_cast<std::uint32_t>(panes.size()));

    std::vector<unsigned char> bytes;
    bytes.push_back('I');
    bytes.push_back('I');
    appendUInt16(bytes, 42);
    appendUInt32(bytes, ifdOffset);

    std::vector<unsigned char> imageBytes;
    auto paneOffset = imageOffset;

    for (std::size_t paneIndex = 0; paneIndex < panes.size(); ++paneIndex) {
        const auto& pixels = panes[paneIndex];
        auto nextIfdOffset = ifdOffset + ifdSize * static_cast<std::uint32_t>(paneIndex + 1);
        if (paneIndex == panes.size() - 1) {
            nextIfdOffset = 0;
        }

        appendUInt16(bytes, ifdEntryCount);
        appendBytes(bytes, ifdEntry(254, 4, 1, 2));
        appendBytes(bytes, ifdEntry(256, 4, 1, tifImageWidth));
        appendBytes(bytes, ifdEntry(257, 4, 1, tifImageHeight));
        appendBytes(bytes, ifdEntry(258, 3, 1, 8));
        appendBytes(bytes, ifdEntry(259, 3, 1, 1));
        appendBytes(bytes, ifdEntry(262, 3, 1, 1));
        appendBytes(bytes, ifdEntry(273, 4, 1, paneOffset));
        appendBytes(bytes, ifdEntry(277, 3, 1, 1));
        appendBytes(bytes, ifdEntry(278, 4, 1, tifImageHeight));
        appendBytes(bytes, ifdEntry(279, 4, 1, static_cast<std::uint32_t>(pixels.size())));
        appendBytes(bytes, ifdEntry(284, 3, 1, 1));
        appendBytes(bytes, ifdEntry(297, 3, 2, static_cast<std::uint32_t>(paneIndex), tifPaneCount));
        appendUInt32(bytes, nextIfdOffset);

        appendBytes(imageBytes, pixels);
        paneOffset += static_cast<std::uint32_t>(pixels.size());
    }

    appendBytes(bytes, imageBytes);
    return bytes;
}

std::shared_ptr<Image> FileTransferDevice::binaryDataBackedImage()
{
    auto image = std::make_shared<Image>();
    image->setWidth(imageWidth).setHeight(imageHeight);
    image->setImageData(makeBinaryData(randomImageBytes()));
    return image;
}

std::shared_ptr<Image> FileTransferDevice::fileHolderBackedImage(const std::vector<unsigned char>& payload,
                                                                 const std::string& extension,
                                                                 unsigned width,
                                                                 unsigned height)
{
    std::shared_ptr<STI::Device::PersistenceManager> persistence;
    if (!getPersistenceManager(persistence) || persistence == nullptr) {
        return {};
    }

    auto file = persistence->makeFileHolder(
        persistence->getTemporaryPath(),
        "fileTransfer-random-image-" + std::to_string(imageMeasurementIndex++) + extension);

    if (!writeBytes(file, payload)) {
        return {};
    }

    auto image = std::make_shared<Image>();
    image->setWidth(width).setHeight(height);
    image->setImageData(file);
    image->setFileID(file->getID());
    return image;
}

std::shared_ptr<Image> FileTransferDevice::rawFileHolderBackedImage()
{
    return fileHolderBackedImage(randomImageBytes(), ".raw");
}

std::shared_ptr<Image> FileTransferDevice::tifFileHolderBackedImage()
{
    return fileHolderBackedImage(randomTifImageBytes(), ".tif", tifImageWidth, tifImageHeight);
}

FileID FileTransferDevice::textFileHolderMeasurement()
{
    std::shared_ptr<STI::Device::PersistenceManager> persistence;
    if (!getPersistenceManager(persistence) || persistence == nullptr) {
        return {};
    }

    auto file = persistence->makeFileHolder(
        persistence->getTemporaryPath(),
        "fileTransfer-text-file-" + std::to_string(fileMeasurementIndex++) + ".txt");

    const std::string text =
        "fileTransfer plain text FileHolder payload\n"
        "This channel returns a MixedValueType.File value, not an Image.\n";

    if (!writeText(file, text)) {
        return {};
    }

    return file->getID();
}

std::shared_ptr<BinaryData> FileTransferDevice::plainBinaryDataMeasurement()
{
    const std::string payload =
        "fileTransfer plain BinaryData payload\n"
        "This channel returns BinaryData that is not an Image.\n";
    return makeBinaryData(std::vector<unsigned char>(payload.begin(), payload.end()));
}

FileID FileTransferDevice::virtualTextFileMeasurement()
{
    std::shared_ptr<STI::Device::PersistenceManager> persistence;
    if (!getPersistenceManager(persistence) || persistence == nullptr) {
        return {};
    }

    FileID fileID;
    fileID.origin = getID().getID();
    fileID.path = "fileTransfer";
    fileID.filename = "fileTransfer-virtual-text-file-" + std::to_string(virtualFileMeasurementIndex++) + ".txt";

    auto file = persistence->makeVirtualFileHolder(fileID);
    const std::string text =
        "fileTransfer virtual text FileHolder payload\n"
        "This channel returns a MixedValueType.File value without writing the source file to disk.\n";

    if (!writeText(file, text)) {
        return {};
    }

    std::shared_ptr<FileServer> fileServer;
    if (!persistence->getFileServer(fileServer) || fileServer == nullptr || !fileServer->addFile(file)) {
        return {};
    }

    return file->getID();
}

std::shared_ptr<Image> FileTransferDevice::invertImageMeasurement(const std::shared_ptr<Image>& image)
{
    if (image == nullptr) {
        return {};
    }

    std::shared_ptr<BinaryData> imageData;
    if (!image->getData(imageData) || imageData == nullptr || !imageData->materialize()) {
        std::cout << "Read ch 19: input image does not have readable BinaryData" << std::endl;
        return {};
    }

    char* pixels = nullptr;
    if (!imageData->getBytes(pixels) || pixels == nullptr) {
        return {};
    }

    std::vector<unsigned char> inverted(imageData->bytes());
    std::transform(pixels, pixels + imageData->bytes(), inverted.begin(), [](char pixel) {
        return static_cast<unsigned char>(255 - static_cast<unsigned char>(pixel));
    });

    auto result = std::make_shared<Image>();
    result->setWidth(image->getWidth()).setHeight(image->getHeight());
    result->setImageData(makeBinaryData(inverted));
    return result;
}

int FileTransferDevice::importedFileSize(const FileID& fileID)
{
    std::shared_ptr<STI::Device::PersistenceManager> persistence;
    std::shared_ptr<FileServer> fileServer;
    if (!getPersistenceManager(persistence)
        || persistence == nullptr
        || !persistence->getFileServer(fileServer)
        || fileServer == nullptr
        || !fileServer->findFile(fileID)) {
        return -1;
    }

    return fileServer->getFileSize(fileID);
}

void FileTransferDevice::imageVectorMeasurement(MixedValue& data)
{
    data.addValue(binaryDataBackedImage());
    data.addValue("image vector result");
    data.addValue(2.5);
}

bool FileTransferDevice::writeChannel(short channel, const MixedValue& value)
{
    if (channel != 6) {
        return false;
    }

    auto fileID = value.getFileID();
    auto fileSize = importedFileSize(fileID);
    if (fileSize < 0) {
        std::cout << "Ch:" << channel << ", uploaded file is not available: " << value.print() << std::endl;
        return false;
    }

    lastImportedFileID = fileID;
    lastImportedFileSize = fileSize;
    std::cout << "Ch:" << channel << ", uploaded file: " << fileSize << " bytes" << std::endl;
    return true;
}

bool FileTransferDevice::readChannel(short channel, const MixedValue& value, MixedValue& data)
{
    switch (channel) {
    case 13:
        data.setValue(binaryDataBackedImage());
        return true;
    case 14:
    {
        auto image = rawFileHolderBackedImage();
        if (image == nullptr) {
            return false;
        }
        data.setValue(image);
        return true;
    }
    case 15:
    {
        auto image = tifFileHolderBackedImage();
        if (image == nullptr) {
            return false;
        }
        data.setValue(image);
        return true;
    }
    case 16:
        data.setValue(textFileHolderMeasurement());
        return true;
    case 17:
        data.setValue(plainBinaryDataMeasurement());
        return true;
    case 18:
        data.setValue(virtualTextFileMeasurement());
        return true;
    case 19:
    {
        auto image = invertImageMeasurement(value.getImage());
        if (image == nullptr) {
            return false;
        }
        data.setValue(image);
        return true;
    }
    case 20:
    {
        auto fileSize = importedFileSize(value.getFileID());
        if (fileSize < 0) {
            return false;
        }
        data.setValue(fileSize);
        return true;
    }
    case 21:
        imageVectorMeasurement(data);
        return true;
    default:
        return false;
    }
}
