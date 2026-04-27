#include "LogCatalogFile.h"

#include "CerealArchives.h"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using STI::Device::LogCatalog;
using STI::Device::LogCatalogEntry;
using STI::Device::LogCatalogFile;


LogCatalogFile::LogCatalogFile(const std::string& filename)
: filename(filename)
{
    load();
}

LogCatalogFile::~LogCatalogFile()
{
}

template<class Archive>
void LogCatalogEntry::serialize(Archive& archive)
{
    archive(
        cereal::make_nvp("logName", logName),
        cereal::make_nvp("firstDay", firstDay),
        cereal::make_nvp("lastDay", lastDay),
        cereal::make_nvp("dayCount", dayCount),
        cereal::make_nvp("fileCount", fileCount),
        cereal::make_nvp("totalBytes", totalBytes),
        cereal::make_nvp("totalLines", totalLines)
        );
}

template<class Archive>
void LogCatalog::serialize(Archive& archive)
{
    archive(
        cereal::make_nvp("deviceID", deviceID),
        cereal::make_nvp("lastUpdate", lastUpdate),
        cereal::make_nvp("logs", logs)
        );
}

void LogCatalogFile::save()
{
    fs::path catalogPath = filename;
    if (catalogPath.has_parent_path() && !fs::exists(catalogPath.parent_path())) {
        fs::create_directories(catalogPath.parent_path());
    }

    std::ofstream file(filename);
    cereal::XMLOutputArchive archive(file);

    archive(logCatalog);
}

void LogCatalogFile::load()
{
    if (!exists()) return;

    std::ifstream file(filename);

    if (!file.is_open()) return;
    if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
        return;
    }

    try {
        cereal::XMLInputArchive archive(file);
        archive(logCatalog);
    }
    catch (const cereal::Exception&) {
        return;
    }
}

bool LogCatalogFile::exists() const
{
    fs::path catalogPath = filename;
    return fs::exists(catalogPath);
}

bool LogCatalogFile::copyCatalog(LogCatalog& catalog)
{
    if (exists()) {
        catalog = logCatalog;
        return true;
    }
    return false;
}

LogCatalog& LogCatalogFile::getCatalog()
{
    return logCatalog;
}

template void LogCatalogEntry::serialize<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);
template void LogCatalogEntry::serialize<cereal::XMLInputArchive>(cereal::XMLInputArchive&);

template void LogCatalog::serialize<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);
template void LogCatalog::serialize<cereal::XMLInputArchive>(cereal::XMLInputArchive&);
