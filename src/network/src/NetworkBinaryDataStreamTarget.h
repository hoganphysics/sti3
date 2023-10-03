#ifndef STI_ENGINE_NETWORKBINARYDATASTREAMTARGET_H
#define STI_ENGINE_NETWORKBINARYDATASTREAMTARGET_H

#include <sti/utils/BinaryDataStream.h>

#include "TBinaryDataStreamTarget_i.h"
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{


class NetworkBinaryDataStreamTarget : public STI::Utils::BinaryDataStreamTarget
{
public:

    NetworkBinaryDataStreamTarget(const std::shared_ptr<STI::Utils::BinaryData>& target);
    NetworkBinaryDataStreamTarget(const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target);
    ~NetworkBinaryDataStreamTarget();

    void start();
    void writeNext(const std::shared_ptr<STI::Utils::BinaryData>& data);
    void stop();

    static bool getTBinaryDataStreamTargetRef(const typename std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& streamTarget,
        STI::TNetwork::TBinaryDataStreamTarget_var& tStreamTarget);


private:

    std::shared_ptr<STI::Utils::BinaryDataStreamTarget> localdataStreamTarget;
    STI::TNetwork::TBinaryDataStreamTarget_i dataStreamTargetServant;
};


} //Network
} //STI

#endif
