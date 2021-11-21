#ifndef STI_ENGINE_SHOT_H
#define STI_ENGINE_SHOT_H


#include <vector>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEvent;
class ShotConfig;


class Shot
{
public:

    virtual ~Shot() {}

    virtual const ShotConfig& getShotConfig() const = 0;
    virtual void getEvents(std::shared_ptr<std::vector<RawEvent>>& evts) = 0;

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
