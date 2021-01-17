
#ifndef STI_DEVICE_GROUPABLEMESSAGE_H
#define STI_DEVICE_GROUPABLEMESSAGE_H


namespace STI
{
namespace Device
{


template<typename Message>
class GroupableMessage
{
public:
//    virtual bool appendMessage(const GroupableMessage<Message>& mess) = 0;

    virtual bool appendMessage(const Message& mess) = 0;

    virtual Message& get() = 0;
};



} // Device
} // STI


#endif

