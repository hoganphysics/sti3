
#include "EngineParseResult.h"
#include <sti/engine/RawEvent.h>

#include "CerealArchives.h"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::EngineParseResult;


template<class Archive>
void EngineParseResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("parsingMessages", messages),
        cereal::make_nvp("parsedDeviceTree", parsedDevices),
        cereal::make_nvp("ParsedEvents", parsedEvents)
        );
}


template void EngineParseResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void EngineParseResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
