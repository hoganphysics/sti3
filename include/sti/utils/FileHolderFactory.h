
#ifndef STI_UTILS_FILEHOLDERFACTORY_H
#define STI_UTILS_FILEHOLDERFACTORY_H

#include <string>
#include <memory>


namespace STI
{
namespace Utils
{

class FileHolder;
class FileID;
class VirtualFileHolder;

class FileHolderFactory
{
public:
    virtual ~FileHolderFactory() {}
    
    virtual std::shared_ptr<FileHolder> makeFileHolder(const std::string& path, const std::string& filename) = 0;
    virtual std::shared_ptr<FileHolder> makeVirtualFileHolder(const FileID& fileID) = 0;
    virtual std::shared_ptr<FileHolder> makeVirtualFileHolder(const std::shared_ptr<VirtualFileHolder>& backingHolder) = 0;

};

} //Utils
} //STI

#endif
