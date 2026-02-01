#ifndef STI_NETWORK_REMOTEBINARYDATASTREAM_H
#define STI_NETWORK_REMOTEBINARYDATASTREAM_H

#include <sti/utils/BinaryDataStream.h>
#include "TReferenceHolder.h"
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{


class RemoteBinaryDataStream : public STI::Utils::BinaryDataStream,
					           public STI::TNetwork::TReferenceHolder<STI::TNetwork::TBinaryDataStream>	//mixin
{
public:

    RemoteBinaryDataStream(::STI::TNetwork::TBinaryDataStream_var dataStream);
    ~RemoteBinaryDataStream();

    void transfer(const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target);

private:

    mutable std::mutex streamMutex;
};


} //Utils
} //STI

#endif
