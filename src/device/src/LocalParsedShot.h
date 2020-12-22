#ifndef STI_ENGINE_LOCALPARSEDSHOT_H
#define STI_ENGINE_LOCALPARSEDSHOT_H

#include "ParsedShot.h"

#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class RawEvent;

class LocalParsedShot : public ParsedShot
{
public:

    LocalParsedShot()
    {
        events = std::make_shared<std::vector<RawEvent>>();
    }
    ~LocalParsedShot()
    {
    }

    void setEvents(const std::shared_ptr<std::vector<RawEvent>>& ets)
    {
        events = ets;
    }

    void getEvents(std::shared_ptr<std::vector<RawEvent>>& ets)
    //void getEvents(std::shared_ptr<std::vector<RawEvent>>& ets)
    {
        ets = events;
    }

private:

    std::shared_ptr<std::vector<RawEvent>> events;

//    std::vector<AbstractEvent> abstractevents;
    //files
    //overwritten vars
    //abstract channel resolution (?)

};


} //Engine
} //STI

#endif
