#ifndef STI_TNETWORK_TFILESERVER_I_H
#define STI_TNETWORK_TFILESERVER_I_H

#include "generated/deviceNet.h"

#include <sti/utils/FileServer.h>

#include <memory>

namespace STI
{
namespace TNetwork
{

class TFileServer_i : public POA_STI::TNetwork::TFileServer
{
public:

	TFileServer_i(STI::Utils::FileServer* fileServer);
	~TFileServer_i();
	
    ::CORBA::Boolean findFile(const ::STI::TNetwork::TFileID& fileID);
    ::CORBA::Boolean getFileSize(const ::STI::TNetwork::TFileID& fileID, ::CORBA::Long& fileSize);
    ::CORBA::Boolean transferFile(const ::STI::TNetwork::TFileID& source, ::STI::TNetwork::TFileHolder_ptr destination, ::STI::TNetwork::TFileTransferType type);
    ::CORBA::Boolean transferFilePartial(const ::STI::TNetwork::TFileID& source, ::STI::TNetwork::TFileHolder_ptr destination, ::CORBA::Long offset, ::CORBA::Long lines);
    ::CORBA::Boolean deleteFile(const ::STI::TNetwork::TFileID& fileID);
      
private:

    STI::Utils::FileServer* localFileServer;
};


} //TNetwork
} //STI


#endif

