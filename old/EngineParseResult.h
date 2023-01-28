
#ifndef STI_ENGINE_ENGINEPARSERESULT_H
#define STI_ENGINE_ENGINEPARSERESULT_H


#include <sti/engine/ParseID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/fwd/RawEvent_fwd.h>

#include <vector>
#include <memory>


namespace STI
{
namespace Engine
{


class EngineParseResult
{
public:

    // ParseID parseID;

    // DeviceEventMap& getParsedEvents();

    DeviceEventMap parsedEvents;
    std::shared_ptr<ParsedDependencyTree> parsedDevices;
    std::vector<EngineParsingMessage> messages;

    template<class Archive>
    void serialize(Archive& archive);

private:

    
};


} //Engine
} //STI

#endif

