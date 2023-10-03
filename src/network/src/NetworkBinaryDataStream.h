#ifndef STI_ENGINE_NETWORKBINARYDATASTREAM_H
#define STI_ENGINE_NETWORKBINARYDATASTREAM_H

#include <sti/utils/BinaryDataStream.h>
#include <sti/utils/BinaryData.h>

#include "TBinaryDataStream_i.h"
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{


class NetworkBinaryDataStream : public STI::Utils::BinaryDataStream
{
public:

    NetworkBinaryDataStream(STI::Utils::BinaryData* data, size_t chunkSize);
    NetworkBinaryDataStream(const std::shared_ptr<STI::Utils::BinaryDataStream>& dataStream);
    ~NetworkBinaryDataStream();

    void transfer(const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target);

    static bool getTBinaryDataStreamRef(const typename std::shared_ptr<STI::Utils::BinaryDataStream>& dataStream,
        STI::TNetwork::TBinaryDataStream_var& tdataStream);

private:

    std::shared_ptr<STI::Utils::BinaryDataStream> localdataStreamTarget;
    STI::TNetwork::TBinaryDataStream_i dataStreamServant;
};


} //Network
} //STI

#endif
