#ifndef STI_PYTHON_LOGBROWSER_H
#define STI_PYTHON_LOGBROWSER_H

#include <sti/device/Device.h>
#include <sti/device/LogID.h>
#include <sti/device/LogManager.h>
#include <sti/device/LogRecord.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/TimeStamp.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace STI
{
namespace Python
{

class LogBrowser
{
public:

    LogBrowser(const std::shared_ptr<STI::Device::LogManager>& sourceLogManager,
        const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer,
        const STI::Device::LogFileRecord& logFileRecord);
    ~LogBrowser();

    const STI::Device::LogID& getLogID() const;
    std::uint64_t getBytes() const;
    std::uint64_t getLineCount() const;
    STI::Utils::TimeStamp getFirstEntryTime() const;
    STI::Utils::TimeStamp getLastEntryTime() const;

    std::size_t getWindowStartLine() const;
    std::size_t getWindowLineCount() const;
    std::string getText() const;

    void refreshMetadata();

    std::string tail(std::size_t lines = 0);
    std::string read(long offset, std::size_t lines = 0);
    std::string pageBackward(std::size_t lines = 0);
    std::string pageForward(std::size_t lines = 0);

    std::string saveLocal(const std::string& path) const;
    std::string repr() const;

private:

    std::size_t normalizedPageLines(std::size_t lines) const;
    std::size_t clippedWindowLineCount(std::size_t startLine, std::size_t requestedLines) const;
    void loadWindow(std::size_t startLine, std::size_t requestedLines, int transferOffset);
    std::string transferText(int offset, int lines) const;

    // Keep both handles alive so paging continues to talk to the origin device.
    std::shared_ptr<STI::Device::LogManager> sourceLogManager;
    std::shared_ptr<STI::Utils::FileServer> sourceFileServer;

    STI::Device::LogFileRecord logFileRecord;

    std::size_t preferredPageLines = 200;
    std::size_t windowStartLine = 0;
    std::size_t windowLineCount = 0;
    std::string currentText;
};

std::shared_ptr<LogBrowser> openLog(
    const std::shared_ptr<STI::Device::Device>& owningDevice,
    const STI::Device::LogID& logID,
    std::size_t tailLines);


} //Python
} //STI

#endif
