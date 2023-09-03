#ifndef STI_DEVICE_LOGFILE_H
#define STI_DEVICE_LOGFILE_H

#include <sti/device/LogID.h>
#include <sti/utils/FileHolder.h>

#include <string>
#include <memory>


namespace STI
{
namespace Device
{


struct LogFile
{
    enum class LogFileType { URL, FileHolder, String };

    LogID id;
    LogFileType type;
    std::string url;
    std::shared_ptr<STI::Utils::FileHolder> fileHolder;
    std::string logString;
};


} //Device
} //STI

#endif
