

#include "RemoteBinaryDataStream.h"
#include "NetworkBinaryDataStreamTarget.h"


using STI::Network::RemoteBinaryDataStream;
using STI::TNetwork::TBinaryDataStream;


RemoteBinaryDataStream::RemoteBinaryDataStream(::STI::TNetwork::TBinaryDataStream_var dataStream)
: STI::TNetwork::TReferenceHolder<TBinaryDataStream>(dataStream)
{
}

RemoteBinaryDataStream::~RemoteBinaryDataStream()
{
}

void RemoteBinaryDataStream::transfer(const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target)
{
	std::unique_lock<std::mutex> streamLock(streamMutex);

	if (isDisabled()) return;

	try {

		STI::TNetwork::TBinaryDataStreamTarget_var tTarget;
		if (!NetworkBinaryDataStreamTarget::getTBinaryDataStreamTargetRef(target, tTarget)) {
			//failed

			auto networkTarget = std::make_shared<STI::Network::NetworkBinaryDataStreamTarget>(target);	//wrap
			
			if (!NetworkBinaryDataStreamTarget::getTBinaryDataStreamTargetRef(networkTarget, tTarget)) {
				return;		//failed again
			}
		}
		
		getTRef()->transfer(tTarget);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

