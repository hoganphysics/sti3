#ifndef STI_NETWORK_REMOTEBINARYDATASTREAMTARGET_H
#define STI_NETWORK_REMOTEBINARYDATASTREAMTARGET_H

#include <sti/utils/BinaryDataStream.h>
#include "TReferenceHolder.h"

#include <memory>


namespace STI
{
namespace Network
{


class RemoteBinaryDataStreamTarget : public STI::Utils::BinaryDataStreamTarget,
					                 public STI::TNetwork::TReferenceHolder<STI::TNetwork::TBinaryDataStreamTarget>	//mixin
{
public:

    RemoteBinaryDataStreamTarget(::STI::TNetwork::TBinaryDataStreamTarget_ptr streamTarget);
    ~RemoteBinaryDataStreamTarget();

    void start();
    void writeNext(const std::shared_ptr<STI::Utils::BinaryData>& data);
    void stop();

private:

    mutable std::mutex streamMutex;
};


} //Utils
} //STI

#endif
