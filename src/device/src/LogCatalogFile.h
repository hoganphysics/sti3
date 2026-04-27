#ifndef STI_DEVICE_LOGCATALOGFILE_H
#define STI_DEVICE_LOGCATALOGFILE_H

#include <sti/utils/TimeStamp.h>

#include <cstdint>
#include <map>
#include <string>


namespace STI
{
namespace Device
{

struct LogCatalogEntry
{
    std::string logName;

    STI::Utils::TimeStamp firstDay;
    STI::Utils::TimeStamp lastDay;

    std::uint64_t dayCount = 0;
    std::uint64_t fileCount = 0;
    std::uint64_t totalBytes = 0;
    std::uint64_t totalLines = 0;

    template<class Archive>
    void serialize(Archive& archive);
};

class LogCatalog
{
public:

    std::string deviceID;
    STI::Utils::TimeStamp lastUpdate;

    std::map<std::string, LogCatalogEntry> logs;

    template<class Archive>
    void serialize(Archive& archive);
};

class LogCatalogFile
{
public:

    explicit LogCatalogFile(const std::string& filename);
    ~LogCatalogFile();

    bool exists() const;
    bool copyCatalog(LogCatalog& catalog);

    LogCatalog& getCatalog();

    void load();
    void save();

private:

    LogCatalog logCatalog;
    std::string filename;
};


} //Device
} //STI

#endif
