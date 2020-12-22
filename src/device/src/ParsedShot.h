#ifndef STI_ENGINE_PARSEDSHOT_H
#define STI_ENGINE_PARSEDSHOT_H


#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class RawEvent;

class ParsedShot
{
public:

    virtual ~ParsedShot() {}

    virtual void getEvents(std::shared_ptr<std::vector<RawEvent>>& ets) = 0;
    //virtual void getEvents(std::vector<RawEvent>& ets) = 0;

private:

//    std::vector<RawEvent> events;

//    std::vector<AbstractEvent> abstractevents;
    //files
    //overwritten vars
    //abstract channel resolution (?)
};


} //Engine
} //STI

#endif
