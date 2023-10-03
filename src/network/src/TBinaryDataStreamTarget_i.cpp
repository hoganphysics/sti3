
#include "TBinaryDataStreamTarget_i.h"
#include "ORBManager.h"
#include "NetworkConvert.h"

#include <sti/utils/BinaryData.h>

using STI::Network::convert;
using STI::TNetwork::TBinaryDataStreamTarget_i;
using ::STI::TNetwork::TBinaryData;
using STI::Utils::BinaryData;


TBinaryDataStreamTarget_i::TBinaryDataStreamTarget_i(STI::Utils::BinaryDataStreamTarget* target)
: localBinaryDataStreamTarget(target)
{
}

TBinaryDataStreamTarget_i::~TBinaryDataStreamTarget_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TBinaryDataStreamTarget_i::start()
{
	if (localBinaryDataStreamTarget != 0) {
		localBinaryDataStreamTarget->start();
	}
}

void TBinaryDataStreamTarget_i::writeNext(const ::STI::TNetwork::TBinaryData& data)
{
	if (localBinaryDataStreamTarget != 0) {
		
		auto binData = std::make_shared<BinaryData>();

		convert<TBinaryData, std::shared_ptr<BinaryData>>(data, binData);

		localBinaryDataStreamTarget->writeNext(binData);
	}
}

void TBinaryDataStreamTarget_i::stop()
{
	if (localBinaryDataStreamTarget != 0) {
		localBinaryDataStreamTarget->stop();
	}
}

