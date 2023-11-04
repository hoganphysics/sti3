#ifndef STI_DEVICE_LOGFILE_H
#define STI_DEVICE_LOGFILE_H

#include <sti/device/LogID.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>

#include <string>
#include <memory>


namespace STI
{
namespace Device
{


struct LogFile
{
    enum class LogFileType { FileID, FileHolder, String };

    LogID id;
    LogFileType type;

    STI::Utils::FileID fileID;
    std::shared_ptr<STI::Utils::FileHolder> fileHolder;
    std::string logString;
};

/* 
switch to:

class LogFile
{
    virtual LogID getLogID() const = 0;
    virtual FileID getFileID() const = 0;
    virtual unsigned size() const = 0;

    virtual void transferFile() = 0;
    
    virtual void transferFile(unsigned offset, unsigned lines) = 0;

    std::string getLogs() const = 0;

};


class LocalLogFile
{

private:

    LogID logID;
    FileID fileID;
};


class RemoteLogFile
{

private:

    LogID logID;
    FileID remoteFileID;
    std::shared_ptr<RemoteFileServer> fileServer;
};

*/


} //Device
} //STI

#endif
