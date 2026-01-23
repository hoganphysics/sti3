#include "RemoteBinaryDataStreamTarget.h"
#include "NetworkConvert.h"

#include <sti/utils/BinaryData.h>

using STI::Network::convert;
using ::STI::TNetwork::TBinaryData;
using STI::Utils::BinaryData;
using STI::Network::RemoteBinaryDataStreamTarget;
using ::STI::TNetwork::TBinaryDataStreamTarget_var;
using ::STI::TNetwork::TBinaryDataStreamTarget;


RemoteBinaryDataStreamTarget::RemoteBinaryDataStreamTarget(TBinaryDataStreamTarget_var streamTarget)
: STI::TNetwork::TReferenceHolder<TBinaryDataStreamTarget>(streamTarget)
{
}

RemoteBinaryDataStreamTarget::~RemoteBinaryDataStreamTarget()
{
}

void RemoteBinaryDataStreamTarget::start()
{
	std::unique_lock<std::mutex> streamLock(streamMutex);

	if (isDisabled()) return;

	try {
		getTRef()->start();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteBinaryDataStreamTarget::writeNext(const std::shared_ptr<STI::Utils::BinaryData>& data)
{
	std::unique_lock<std::mutex> streamLock(streamMutex);

	if (isDisabled()) return;

	if (data == 0) return;

	try {
		TBinaryData tData;
		convert<std::shared_ptr<BinaryData>, TBinaryData>(data, tData);
		getTRef()->writeNext(tData);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteBinaryDataStreamTarget::stop()
{
	std::unique_lock<std::mutex> streamLock(streamMutex);

	if (isDisabled()) return;

	try {
		getTRef()->stop();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


