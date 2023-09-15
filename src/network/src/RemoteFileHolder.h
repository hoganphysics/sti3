#ifndef STI_UTILS_REMOTEFILEHOLDER_H
#define STI_UTILS_REMOTEFILEHOLDER_H

#include <sti/utils/FileHolder.h>
#include <sti/utils/FileID.h>
#include "TReferenceHolder.h"
#include "TFileHolderRefInterface.h"
#include <sti/utils/CachedValue.h>

#include <string>
#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{


class RemoteFileHolder : public STI::Utils::FileHolder,
					     public STI::TNetwork::TReferenceHolder<STI::TNetwork::TFileHolder>,	//mixin
                         public STI::Network::TFileHolderRefInterface	//mixin
{
public:

    RemoteFileHolder(::STI::TNetwork::TFileHolder_ptr fileHolder);
    ~RemoteFileHolder();

    STI::Utils::FileID getID() const;
    std::string getFilename() const;
    unsigned getFileSize() const;
    bool exists() const;

    bool transferFile(const std::shared_ptr<STI::Utils::FileHolder>& destination);
    unsigned maxBufferSize() const;

    std::string md5Checksum();

    bool openFile();
    bool write(const char* buffer, unsigned length);
    void closeFile();

private:

    bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder);

    //Cached data is mutable because it's the servant's data that's const, not this remote reference.
    mutable STI::Utils::CachedValue<std::string> filename;
    mutable STI::Utils::CachedValue<bool> fileExists;
    mutable STI::Utils::CachedValue<unsigned> bufferSize;
    mutable STI::Utils::CachedValue<std::string> checksum;

    mutable std::mutex fileMutex;

};


} //Utils
} //STI

#endif
