#ifndef STI_UTILS_VIRTUALFILEHOLDER_H
#define STI_UTILS_VIRTUALFILEHOLDER_H

#include <sti/utils/CachedValue.h>
#include <sti/utils/LocalFileHolder.h>

#include <sti/utils/FileServer.h>

#include <string>
#include <memory>
#include <mutex>
#include <sstream>

namespace STI
{
namespace Utils
{


class VirtualFileHolder : public LocalFileHolder
{
public:

    VirtualFileHolder(const std::string& originID, const FileID& fileID);

    ~VirtualFileHolder();

    std::ostream* getostream();
    bool getistream(std::shared_ptr<std::istream>& istream);

    bool openFile();
    void closeFile();

private:

    FileID fileID;
    // unsigned size;

    std::shared_ptr<std::stringstream> data;

    mutable std::mutex fileMutex;
};



} //Utils
} //STI

#endif
