#ifndef STI_UTILS_LOCALBINARYDATASTREAM_H
#define STI_UTILS_LOCALBINARYDATASTREAM_H

#include <sti/utils/BinaryDataStream.h>

#include <memory>
#include <vector>


namespace STI
{
namespace Utils
{

class BinaryData;


class LocalBinaryDataStreamTarget : public BinaryDataStreamTarget
{
public:

    LocalBinaryDataStreamTarget(const std::shared_ptr<BinaryData>& target);
    ~LocalBinaryDataStreamTarget();

    void start();
    void writeNext(const std::shared_ptr<BinaryData>& data);
    void stop();

private:

    std::shared_ptr<BinaryData> target;
    std::vector<std::shared_ptr<BinaryData>> chunks;
};


class LocalBinaryDataStream : public BinaryDataStream
{
public:

    LocalBinaryDataStream(BinaryData* data, size_t chunkSize);
    ~LocalBinaryDataStream();

    void transfer(const std::shared_ptr<BinaryDataStreamTarget>& target);

private:

    BinaryData* data;
    size_t chunkSize;
};


} //Utils
} //STI

#endif
