
#include "TBinaryDataStream_i.h"
// #include "ORBManager.h"
#include "RemoteBinaryDataStreamTarget.h"

using STI::TNetwork::TBinaryDataStream_i;


TBinaryDataStream_i::TBinaryDataStream_i(STI::Utils::BinaryDataStream* dataStream)
: localBinaryDataStream(dataStream)
{
}

TBinaryDataStream_i::~TBinaryDataStream_i()
{
	// STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TBinaryDataStream_i::transfer(::STI::TNetwork::TBinaryDataStreamTarget_ptr target)
{
	if (localBinaryDataStream == 0) return;
	if (CORBA::is_nil(target)) return;

	STI::TNetwork::TBinaryDataStreamTarget_var target_var = STI::TNetwork::TBinaryDataStreamTarget::_duplicate(target);
	std::shared_ptr<STI::Utils::BinaryDataStreamTarget> remoteTarget = 
		std::make_shared<STI::Network::RemoteBinaryDataStreamTarget>(target_var);

	localBinaryDataStream->transfer(remoteTarget);
}

