#ifndef STI_UTILS_BINARYDATASTREAM_H
#define STI_UTILS_BINARYDATASTREAM_H

#include <memory>


namespace STI
{
namespace Utils
{

class BinaryData;


class BinaryDataStreamTarget    //remote
{
public:

    ~BinaryDataStreamTarget() {}

    virtual void start() = 0;
    virtual void writeNext(const std::shared_ptr<BinaryData>& data) = 0;
    virtual void stop() = 0;
};


class BinaryDataStream  //host
{
public:

    ~BinaryDataStream() {}

    virtual void transfer(const std::shared_ptr<BinaryDataStreamTarget>& target) = 0;
};


} //Utils
} //STI

#endif
