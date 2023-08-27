#ifndef STI_DEVICE_LOGRECORDFILE_H
#define STI_DEVICE_LOGRECORDFILE_H


#include <sti/device/LogRecord.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/ConfigFile.h>
#include <sti/utils/TimeStamp.h>

#include <vector>
#include <map>

namespace STI
{
namespace Device
{

class LogRecordFile
{
public:

    LogRecordFile(const std::string& filename);
    ~LogRecordFile();

    bool exists() const;
    void setLogStatus(const DeviceID& id, LogRecordStatus status);
    bool copyRecord(LogRecord& record);

    LogRecord& getRecord();

    void load();
    void save();

private:

    LogRecord logRecord;
    std::string filename;

};


} //Device
} //STI

#endif
