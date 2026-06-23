#ifndef STI_DEVICE_IMPORTEDFILE_H
#define STI_DEVICE_IMPORTEDFILE_H

#include <sti/utils/FileID.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace STI
{
namespace Device
{

enum class ImportStorage
{
    DiskTemporary,
    Virtual
};

enum class ImportCollisionPolicy
{
    Unique,
    FailIfExists,
    Replace
};

enum class ImportLifetime
{
    Handle
};

struct ImportFileOptions
{
    ImportStorage storage = ImportStorage::DiskTemporary;
    ImportCollisionPolicy collision = ImportCollisionPolicy::Unique;
    ImportLifetime lifetime = ImportLifetime::Handle;
    std::chrono::seconds ttl = std::chrono::minutes(10);
};

class ImportedFile
{
public:
    using ReleaseCallback = std::function<bool(const std::string&)>;

    ImportedFile(const std::string& importID,
                 const STI::Utils::FileID& fileID,
                 ReleaseCallback releaseCallback);

    ~ImportedFile();

    ImportedFile(const ImportedFile&) = delete;
    ImportedFile& operator=(const ImportedFile&) = delete;

    const std::string& getImportID() const;
    const STI::Utils::FileID& getFileID() const;
    bool close();
    bool isClosed() const;

private:
    std::string importID;
    STI::Utils::FileID fileID;
    ReleaseCallback releaseCallback;
    bool closed;
};

} //Device
} //STI

#endif
