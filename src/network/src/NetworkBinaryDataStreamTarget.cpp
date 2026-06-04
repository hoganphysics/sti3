

#include "NetworkBinaryDataStreamTarget.h"
#include "LocalBinaryDataStream.h"

#include "ORBManager.h"

using STI::Network::NetworkBinaryDataStreamTarget;


NetworkBinaryDataStreamTarget::NetworkBinaryDataStreamTarget(const std::shared_ptr<STI::Utils::BinaryData>& target)
: NetworkBinaryDataStreamTarget((std::shared_ptr<STI::Utils::BinaryDataStreamTarget>) nullptr)
{
	localdataStreamTarget = std::make_shared<STI::Utils::LocalBinaryDataStreamTarget>(target);
}

NetworkBinaryDataStreamTarget::NetworkBinaryDataStreamTarget(const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target)
: dataStreamTargetServantHolder(new STI::TNetwork::TBinaryDataStreamTarget_i(this))
{
	localdataStreamTarget = target;
}

NetworkBinaryDataStreamTarget::~NetworkBinaryDataStreamTarget()
{
}

void NetworkBinaryDataStreamTarget::start()
{
	if (localdataStreamTarget != 0) {
		localdataStreamTarget->start();
	}
}

void NetworkBinaryDataStreamTarget::writeNext(const std::shared_ptr<STI::Utils::BinaryData>& data)
{
	if (localdataStreamTarget != 0) {
		localdataStreamTarget->writeNext(data);
	}
}

void NetworkBinaryDataStreamTarget::stop()
{
	if (localdataStreamTarget != 0) {
		localdataStreamTarget->stop();
	}
}

bool NetworkBinaryDataStreamTarget::getTBinaryDataStreamTargetRef(
	const typename std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& streamTarget, 
	STI::TNetwork::TBinaryDataStreamTarget_var& tStreamTarget)
{
	std::shared_ptr<NetworkBinaryDataStreamTarget> networkStreamTarget;
	networkStreamTarget = std::dynamic_pointer_cast<NetworkBinaryDataStreamTarget>(streamTarget);

	if (networkStreamTarget == 0) return false;		//check dynamic_pointer_cast

	auto ref = networkStreamTarget->dataStreamTargetServantHolder.getRefVar();
	if (CORBA::is_nil(ref)) {
		return false;
	}

	tStreamTarget = ref._retn();
	return !CORBA::is_nil(tStreamTarget);
}
