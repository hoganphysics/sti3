
#ifndef STI_UTILS_REMOTEFILEHOLDER_H
#define STI_UTILS_REMOTEFILEHOLDER_H

#include "utils/FileHolder.h"
#include "TReferenceHolder.h"
#include "TFileHolderRefInterface.h"

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

    std::string getFilename() const;
    bool exists() const;

    bool transferFile(const std::shared_ptr<STI::Utils::FileHolder>& destination);
    unsigned maxBufferSize() const;

    bool deleteFile();

    std::string md5Checksum();

    bool openFile();
    bool write(const char* buffer, unsigned length);
    void closeFile();

private:

    bool getTFileHolderRef(STI::TNetwork::TFileHolder_var& tFileHolder);

    mutable std::mutex fileMutex;

};


} //Utils
} //STI

#endif
