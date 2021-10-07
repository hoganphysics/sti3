
#include "ParseID.h"

#include "CerealArchives.h"
#include <cereal/types/string.hpp>


using STI::Engine::ParseID;
using STI::Engine::EngineJobSourceID;

ParseID::ParseID()
{
    targetEnginePool = 1;   //1=common shot pool, 0=async pool (readChannel/writeChannel)
}

template<class Archive>
void EngineJobSourceID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("user", user), 
            cereal::make_nvp("machine", machine));
}

template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template<class Archive>
void ParseID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseTimestamp", parseTimestamp), 
            cereal::make_nvp("file", file), 
            cereal::make_nvp("jobSourceID", jobSourceID), 
            cereal::make_nvp("comment", comment));
}

template void STI::Engine::ParseID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ParseID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

