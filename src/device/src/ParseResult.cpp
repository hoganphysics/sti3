#include <sti/engine/ParseResult.h>

#include <sti/engine/ParseID.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/utils/FileHolder.h>

#include "ParsedDependencyTree.h"
#include "RawEventGroup.h"

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::ParseResult;


ParseResult::ParseResult()
{
}

ParseResult::~ParseResult()
{
}

template<class Archive>
void ParseResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("pid", pid),
        cereal::make_nvp("baseEventGroup", baseEventGroup), 
        cereal::make_nvp("parsedDevices", parsedDevices),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("stackTraceResult", stackTraceResult)
        );
}


template void ParseResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParseResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
