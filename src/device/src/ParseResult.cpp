
#include "ParseResult.h"


#include "RawEventGroup.h"


#include <sti/engine/ParseID.h>
#include <sti/engine/RawEvent.h>
#include <sti/utils/FileHolder.h>


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
        cereal::make_nvp("TimingFiles", timingFiles),
        cereal::make_nvp("functionNames", functionNames), 
        cereal::make_nvp("eventGroups", eventGroups),
        cereal::make_nvp("parsedVars", parsedVars),
        cereal::make_nvp("parsedTags", parsedTags)
        );
}


template void ParseResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParseResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
