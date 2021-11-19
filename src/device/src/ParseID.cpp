
#include "ParseID.h"
#include "EngineJobSourceID.h"

#include "CerealArchives.h"
#include <cereal/types/string.hpp>


using STI::Engine::ParseID;
using STI::Engine::EngineJobSourceID;

ParseID::ParseID()
{
}


template<class Archive>
void ParseID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseTimestamp", parseTimestamp),
            cereal::make_nvp("ShotConfig", shotConfig));

}

template void STI::Engine::ParseID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ParseID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

