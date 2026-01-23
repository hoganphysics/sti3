
#include "NetworkBinaryDataStream.h"
#include "LocalBinaryDataStream.h"
#include "ORBManager.h"

using STI::Network::NetworkBinaryDataStream;
using STI::Utils::BinaryDataStreamTarget;


NetworkBinaryDataStream::NetworkBinaryDataStream(STI::Utils::BinaryData* data, size_t chunkSize)
: NetworkBinaryDataStream((std::shared_ptr<STI::Utils::BinaryDataStream>) nullptr)
{
	localdataStreamTarget = std::make_shared<STI::Utils::LocalBinaryDataStream>(data, chunkSize);
}

NetworkBinaryDataStream::NetworkBinaryDataStream(const std::shared_ptr<STI::Utils::BinaryDataStream>& dataStream)
: dataStreamServantHolder(new STI::TNetwork::TBinaryDataStream_i(this))
{
	localdataStreamTarget = dataStream;
}

NetworkBinaryDataStream::~NetworkBinaryDataStream()
{
}

void NetworkBinaryDataStream::transfer(const std::shared_ptr<BinaryDataStreamTarget>& target)
{
	if (localdataStreamTarget != 0) {
		localdataStreamTarget->transfer(target);
	}
}

bool NetworkBinaryDataStream::getTBinaryDataStreamRef(const typename std::shared_ptr<STI::Utils::BinaryDataStream>& dataStream,
	STI::TNetwork::TBinaryDataStream_var& tdataStream)
{
	std::shared_ptr<NetworkBinaryDataStream> networkDataStream;
	networkDataStream = std::dynamic_pointer_cast<NetworkBinaryDataStream>(dataStream);

	if (networkDataStream == 0) return false;		//check dynamic_pointer_cast

	tdataStream = networkDataStream->dataStreamServantHolder.getRefVar();
	return !CORBA::is_nil(tdataStream);
}

