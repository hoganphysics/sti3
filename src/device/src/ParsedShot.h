#ifndef STI_ENGINE_PARSEDSHOT_H
#define STI_ENGINE_PARSEDSHOT_H


#include <vector>


namespace STI
{
namespace Engine
{

class RawEvent;

class ParsedShot
{
public:
//    std::vector<AbstractEvent> abstractevents;
    std::vector<RawEvent> events;
    //files
    //overwritten vars
    //abstract channel resolution (?)
};


} //Engine
} //STI

#endif
