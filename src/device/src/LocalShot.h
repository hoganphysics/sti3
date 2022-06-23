#ifndef STI_ENGINE_LOCALSHOT_H
#define STI_ENGINE_LOCALSHOT_H

#include "Shot.h"
#include <sti/engine/ShotConfig.h>

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

    LocalShot(const ShotConfig& config)
    : shotConfig(config)
    {
        events = std::make_shared<std::vector<RawEvent>>();
    }
    virtual ~LocalShot()
    {
    }

    const ShotConfig& getShotConfig() const
    {
        return shotConfig;
    }

    void setEvents(const std::shared_ptr<std::vector<RawEvent>>& evts)
    {
        events = evts;
    }

    void getEvents(std::shared_ptr<std::vector<RawEvent>>& evts)
    {
        evts = events;
    }

    void addEvent(const RawEvent& evt)
    {
        if (events != 0) {
            events->push_back(evt);
        }
    }

private:

    ShotConfig shotConfig;

    std::shared_ptr<std::vector<RawEvent>> events;

//    std::vector<AbstractEvent> abstractevents;
    //files
    //overwritten vars
    //abstract channel resolution (?)

};


} //Engine
} //STI

#endif
