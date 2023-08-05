
#ifndef STI_UTILS_FILEHOLDERFACTORY_H
#define STI_UTILS_FILEHOLDERFACTORY_H

#include <string>
#include <memory>


namespace STI
{
namespace Utils
{

class FileHolder;

class FileHolderFactory
{
public:
    virtual ~FileHolderFactory() {}
    
    virtual std::shared_ptr<FileHolder> makeFileHolder(const std::string& filename) = 0;

};

} //Utils
} //STI

#endif
