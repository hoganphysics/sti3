
#ifndef STI_NETWORK_CONVERT_FILE_H
#define STI_NETWORK_CONVERT_FILE_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <sti/utils/FileID.h>
#include <sti/utils/FileServer.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Device
{


} //Device



//FileID
template<>
Utils::FileID Network::convert<TNetwork::TFileID, Utils::FileID>(const TNetwork::TFileID& tFileID);
template<>
TNetwork::TFileID Network::convert<Utils::FileID, TNetwork::TFileID>(const Utils::FileID& fileID);

template<>
bool Network::convert<TNetwork::TFileID, Utils::FileID>(const TNetwork::TFileID& tFileID, Utils::FileID& fileID);
template<>
bool Network::convert<Utils::FileID, TNetwork::TFileID>(const Utils::FileID& fileID, TNetwork::TFileID& tFileID);



//FileTransferType
template<>
Utils::FileTransferType Network::convert<TNetwork::TFileTransferType, Utils::FileTransferType>(const TNetwork::TFileTransferType& tType);
template<>
TNetwork::TFileTransferType Network::convert<Utils::FileTransferType, TNetwork::TFileTransferType>(const Utils::FileTransferType& type);



//FileServer
template<>
bool Network::convert<TNetwork::TFileServer_var, std::shared_ptr<Utils::FileServer>>(
        const TNetwork::TFileServer_var& tFileServer, std::shared_ptr<Utils::FileServer>& fileServer);
template<>
bool Network::convert<std::shared_ptr<Utils::FileServer>, TNetwork::TFileServer_var>(
        const std::shared_ptr<Utils::FileServer>& fileServer, TNetwork::TFileServer_var& tFileServer);



} //STI

#endif

