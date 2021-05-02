#ifndef STI_ENGINE_LOCALSHOT_H
#define STI_ENGINE_LOCALSHOT_H

#include "Shot.h"

#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class RawEvent;

class LocalShot : public Shot
{
public:

    LocalShot()
    {
        events = std::make_shared<std::vector<RawEvent>>();
    }
    ~LocalShot()
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
