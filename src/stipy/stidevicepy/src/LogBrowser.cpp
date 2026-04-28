#include "LogBrowser.h"

#include <sti/device/DeviceCollection.h>
#include <sti/device/PersistenceManager.h>
#include <sti/utils/VirtualFileHolder.h>

#include <algorithm>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

using STI::Device::Device;
using STI::Device::DeviceCollection;
using STI::Device::LogFileRecord;
using STI::Device::LogID;
using STI::Device::LogManager;
using STI::Device::LogRecord;
using STI::Device::PersistenceManager;
using STI::Python::LogBrowser;
using STI::Utils::FileHolder;
using STI::Utils::FileHolderFactory;
using STI::Utils::FileServer;
using STI::Utils::FileTransferType;
using STI::Utils::TimeStamp;
using STI::Utils::VirtualFileHolder;

namespace {

std::string describeLogID(const LogID& logID)
{
    std::ostringstream stream;
    stream << logID.deviceID.getID()
           << " | " << logID.date
           << " | " << logID.logName
           << " | " << logID.index;
    return stream.str();
}

bool lookupLogFileRecord(LogManager& manager, const LogID& logID, LogFileRecord& fileRecord)
{
    LogRecord record;
    if (!manager.getLogRecord(logID.date, record)) {
        return false;
    }

    const auto deviceIt = record.deviceLogRecords.find(logID.deviceID.getID());
    if (deviceIt == record.deviceLogRecords.end()) {
        return false;
    }

    const auto logIt = deviceIt->second.logs.find(logID.logName);
    if (logIt == deviceIt->second.logs.end()) {
        return false;
    }

    const auto fileIt = logIt->second.files.find(logID.index);
    if (fileIt == logIt->second.files.end()) {
        return false;
    }

    fileRecord = fileIt->second;
    fileRecord.id = logID;
    return true;
}

std::shared_ptr<Device> resolveSourceDevice(
    const std::shared_ptr<Device>& owningDevice,
    const STI::Device::DeviceID& sourceDeviceID)
{
    if (owningDevice == nullptr) {
        return nullptr;
    }

    if (owningDevice->getID() == sourceDeviceID) {
        return owningDevice;
    }

    std::shared_ptr<DeviceCollection> collection;
    owningDevice->getCollection(collection);
    if (collection == nullptr) {
        return nullptr;
    }

    std::shared_ptr<Device> sourceDevice;
    if (!collection->get(sourceDeviceID, sourceDevice) || sourceDevice == nullptr) {
        return nullptr;
    }

    return sourceDevice;
}

std::string readVirtualHolderText(const std::shared_ptr<VirtualFileHolder>& virtualHolder)
{
    if (virtualHolder == nullptr) {
        return "";
    }

    std::shared_ptr<std::istream> stream;
    if (!virtualHolder->getistream(stream) || stream == nullptr) {
        return "";
    }

    stream->clear();
    stream->seekg(0, std::ios::beg);

    std::ostringstream text;
    text << stream->rdbuf();
    return text.str();
}

std::string outputDirectoryForPath(const fs::path& outputPath)
{
    const auto parent = outputPath.parent_path();
    return parent.empty() ? "." : parent.string();
}

int checkedLineCount(std::size_t lines)
{
    if (lines > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("Requested log page is too large.");
    }
    return static_cast<int>(lines);
}

int checkedOffset(std::size_t offset)
{
    if (offset > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("Requested log offset is too large.");
    }
    return static_cast<int>(offset);
}

} // namespace


LogBrowser::LogBrowser(const std::shared_ptr<LogManager>& sourceLogManager,
    const std::shared_ptr<FileServer>& sourceFileServer,
    const std::shared_ptr<FileHolderFactory>& destinationFileHolderFactory,
    const LogFileRecord& logFileRecord)
: sourceLogManager(sourceLogManager),
  sourceFileServer(sourceFileServer),
  destinationFileHolderFactory(destinationFileHolderFactory),
  logFileRecord(logFileRecord)
{
}

LogBrowser::~LogBrowser()
{
}

const LogID& LogBrowser::getLogID() const
{
    return logFileRecord.id;
}

std::uint64_t LogBrowser::getBytes() const
{
    return logFileRecord.bytes;
}

std::uint64_t LogBrowser::getLineCount() const
{
    return logFileRecord.lineCount;
}

TimeStamp LogBrowser::getFirstEntryTime() const
{
    return logFileRecord.firstEntryTime;
}

TimeStamp LogBrowser::getLastEntryTime() const
{
    return logFileRecord.lastEntryTime;
}

std::size_t LogBrowser::getWindowStartLine() const
{
    return windowStartLine;
}

std::size_t LogBrowser::getWindowLineCount() const
{
    return windowLineCount;
}

std::string LogBrowser::getText() const
{
    return currentText;
}

std::size_t LogBrowser::normalizedPageLines(std::size_t lines) const
{
    if (lines != 0) {
        return lines;
    }

    if (windowLineCount != 0) {
        return windowLineCount;
    }

    return preferredPageLines == 0 ? 200 : preferredPageLines;
}

std::size_t LogBrowser::clippedWindowLineCount(std::size_t startLine, std::size_t requestedLines) const
{
    if (startLine >= static_cast<std::size_t>(logFileRecord.lineCount)) {
        return 0;
    }

    const auto available = static_cast<std::size_t>(logFileRecord.lineCount) - startLine;
    return std::min(requestedLines, available);
}

void LogBrowser::loadWindow(std::size_t startLine, std::size_t requestedLines, int transferOffset)
{
    preferredPageLines = requestedLines;
    windowStartLine = startLine;
    windowLineCount = clippedWindowLineCount(startLine, requestedLines);

    if (windowLineCount == 0) {
        currentText.clear();
        return;
    }

    currentText = transferText(transferOffset, checkedLineCount(windowLineCount));
}

std::string LogBrowser::transferText(int offset, int lines) const
{
    if (sourceFileServer == nullptr) {
        throw std::runtime_error("Log browser has no source file server.");
    }
    if (destinationFileHolderFactory == nullptr) {
        throw std::runtime_error("Log browser has no destination file holder factory.");
    }

    auto virtualHolder = std::make_shared<VirtualFileHolder>(
        logFileRecord.id.deviceID.getID(),
        logFileRecord.fileID);
    auto destination = destinationFileHolderFactory->makeVirtualFileHolder(virtualHolder);

    if (destination == nullptr) {
        throw std::runtime_error("Failed to allocate a log paging buffer.");
    }

    if (!sourceFileServer->transferFilePartial(logFileRecord.fileID, destination, offset, lines)) {
        throw std::runtime_error("Failed to read log text from source device: " + describeLogID(logFileRecord.id));
    }

    return readVirtualHolderText(virtualHolder);
}

void LogBrowser::refreshMetadata()
{
    if (sourceLogManager == nullptr) {
        throw std::runtime_error("Log browser has no source log manager.");
    }

    LogFileRecord refreshed;
    if (!lookupLogFileRecord(*sourceLogManager, logFileRecord.id, refreshed)) {
        throw std::runtime_error("Failed to refresh metadata for log: " + describeLogID(logFileRecord.id));
    }

    logFileRecord = refreshed;

    if (windowStartLine > static_cast<std::size_t>(logFileRecord.lineCount)) {
        windowStartLine = static_cast<std::size_t>(logFileRecord.lineCount);
    }
    windowLineCount = clippedWindowLineCount(windowStartLine, std::max(windowLineCount, preferredPageLines));
}

std::string LogBrowser::tail(std::size_t lines)
{
    const auto requestedLines = normalizedPageLines(lines);
    refreshMetadata();

    const auto actualLines = std::min<std::size_t>(
        requestedLines,
        static_cast<std::size_t>(logFileRecord.lineCount));
    const auto startLine = (static_cast<std::size_t>(logFileRecord.lineCount) > actualLines)
        ? static_cast<std::size_t>(logFileRecord.lineCount) - actualLines
        : 0;

    const int transferOffset = actualLines == 0 ? 0 : -checkedLineCount(actualLines);
    loadWindow(startLine, requestedLines, transferOffset);
    return currentText;
}

std::string LogBrowser::read(long offset, std::size_t lines)
{
    const auto requestedLines = normalizedPageLines(lines);

    if (offset < 0) {
        const auto distanceFromEnd = static_cast<std::size_t>(-offset);
        const auto startLine = (static_cast<std::size_t>(logFileRecord.lineCount) > distanceFromEnd)
            ? static_cast<std::size_t>(logFileRecord.lineCount) - distanceFromEnd
            : 0;
        loadWindow(startLine, requestedLines, static_cast<int>(offset));
        return currentText;
    }

    const auto startLine = std::min<std::size_t>(
        static_cast<std::size_t>(offset),
        static_cast<std::size_t>(logFileRecord.lineCount));
    loadWindow(startLine, requestedLines, checkedOffset(startLine));
    return currentText;
}

std::string LogBrowser::pageBackward(std::size_t lines)
{
    const auto requestedLines = normalizedPageLines(lines);
    const auto startLine = (windowStartLine > requestedLines)
        ? windowStartLine - requestedLines
        : 0;

    loadWindow(startLine, requestedLines, checkedOffset(startLine));
    return currentText;
}

std::string LogBrowser::pageForward(std::size_t lines)
{
    const auto requestedLines = normalizedPageLines(lines);
    const auto maxStartLine = (static_cast<std::size_t>(logFileRecord.lineCount) > requestedLines)
        ? static_cast<std::size_t>(logFileRecord.lineCount) - requestedLines
        : 0;
    const auto desiredStartLine = windowStartLine + requestedLines;
    const auto startLine = std::min(desiredStartLine, maxStartLine);

    loadWindow(startLine, requestedLines, checkedOffset(startLine));
    return currentText;
}

std::string LogBrowser::saveLocal(const std::string& path) const
{
    if (path.empty()) {
        throw std::runtime_error("saveLocal requires a destination path.");
    }

    if (sourceFileServer == nullptr) {
        throw std::runtime_error("Log browser has no source file server.");
    }
    if (destinationFileHolderFactory == nullptr) {
        throw std::runtime_error("Log browser has no destination file holder factory.");
    }

    fs::path outputPath(path);
    if (fs::exists(outputPath) && fs::is_directory(outputPath)) {
        outputPath /= logFileRecord.fileID.filename;
    }

    auto destination = destinationFileHolderFactory->makeFileHolder(
        outputDirectoryForPath(outputPath),
        outputPath.filename().string());

    if (destination == nullptr) {
        throw std::runtime_error("Failed to allocate a destination file for saveLocal.");
    }

    if (!sourceFileServer->transferFile(logFileRecord.fileID, destination, FileTransferType::Binary)) {
        throw std::runtime_error("Failed to save log locally: " + describeLogID(logFileRecord.id));
    }

    return outputPath.string();
}

std::string LogBrowser::repr() const
{
    std::ostringstream stream;
    stream << "<LogBrowser | "
           << logFileRecord.id.deviceID.getID()
           << " | " << logFileRecord.id.logName
           << " | " << logFileRecord.id.date
           << " | " << logFileRecord.id.index
           << " | " << logFileRecord.bytes << " bytes"
           << " | " << logFileRecord.lineCount << " lines";

    if (windowLineCount != 0) {
        stream << " | view " << (windowStartLine + 1)
               << "-" << (windowStartLine + windowLineCount);
    }

    stream << ">";

    if (!currentText.empty()) {
        stream << "\n" << currentText;
    }

    return stream.str();
}

std::shared_ptr<STI::Python::LogBrowser> STI::Python::openLog(
    const std::shared_ptr<STI::Device::Device>& owningDevice,
    const STI::Device::LogID& logID,
    std::size_t tailLines)
{
    auto sourceDevice = resolveSourceDevice(owningDevice, logID.deviceID);
    if (sourceDevice == nullptr) {
        throw std::runtime_error(
            "Could not resolve the originating device for log open. "
            "The device must still be present in the current collection: "
            + describeLogID(logID));
    }

    std::shared_ptr<LogManager> sourceLogManager;
    if (!sourceDevice->getLogManager(sourceLogManager) || sourceLogManager == nullptr) {
        throw std::runtime_error("Could not get the source log manager for log: " + describeLogID(logID));
    }

    LogFileRecord logFileRecord;
    if (!lookupLogFileRecord(*sourceLogManager, logID, logFileRecord)) {
        throw std::runtime_error("Could not resolve log metadata for log: " + describeLogID(logID));
    }

    std::shared_ptr<PersistenceManager> persistenceManager;
    if (!sourceDevice->getPersistenceManager(persistenceManager) || persistenceManager == nullptr) {
        throw std::runtime_error("Could not get the source persistence manager for log: " + describeLogID(logID));
    }

    std::shared_ptr<FileServer> sourceFileServer;
    if (!persistenceManager->getFileServer(sourceFileServer) || sourceFileServer == nullptr) {
        throw std::runtime_error("Could not get the source file server for log: " + describeLogID(logID));
    }

    auto browser = std::make_shared<LogBrowser>(sourceLogManager, sourceFileServer, persistenceManager, logFileRecord);
    browser->tail(tailLines);
    return browser;
}
