#ifndef STI_NETWORK_REMOTEFILESERVER_H
#define STI_NETWORK_REMOTEFILESERVER_H

#include <sti/utils/FileServer.h>
#include "TReferenceHolder.h"
#include "TFileServerRefInterface.h"
#include <sti/utils/CachedValue.h>

#include <string>
#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{


class RemoteFileServer : public STI::Utils::FileServer,
					     public STI::TNetwork::TReferenceHolder<STI::TNetwork::TFileServer>,	//mixin
                         public STI::Network::TFileServerRefInterface	//mixin
{
public:

    RemoteFileServer(::STI::TNetwork::TFileServer_var fileServer);
    ~RemoteFileServer();

	bool findFile(const STI::Utils::FileID& fileID);
	int getFileSize(const STI::Utils::FileID& fileID);
	bool transferFile(const STI::Utils::FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, STI::Utils::FileTransferType type);
	bool transferFilePartial(const STI::Utils::FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, int offset, int lines);
	bool deleteFile(const STI::Utils::FileID& fileID);

private:

    bool getTFileServerRef(STI::TNetwork::TFileServer_var& tFileServer);

    mutable std::mutex fileServerMutex;
};


} //Utils
} //STI

#endif


