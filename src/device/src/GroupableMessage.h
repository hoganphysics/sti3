
#ifndef STI_UTILS_GROUPABLEMESSAGE_H
#define STI_UTILS_GROUPABLEMESSAGE_H


namespace STI
{
namespace Utils
{


template<typename Message>
class GroupableMessage
{
public:
//    virtual bool appendMessage(const GroupableMessage<Message>& mess) = 0;

    virtual bool appendMessage(const Message& mess) = 0;

    virtual Message& get() = 0;
};



} // UTILS
} // STI


#endif

