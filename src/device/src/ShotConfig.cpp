
#include "ShotConfig.h"

using STI::Engine::ShotConfig;

#include "CerealArchives.h"
#include <cereal/types/string.hpp>


ShotConfig::ShotConfig()
{
    targetEnginePool = 1;   //1=common shot pool, 0=async pool (readChannel/writeChannel)
}

template<class Archive>
void ShotConfig::serialize(Archive& archive)
{
    archive(cereal::make_nvp("shotType", shotType), 
            cereal::make_nvp("jobSourceID", jobSourceID), 
            cereal::make_nvp("targetEnginePool", targetEnginePool), 
            cereal::make_nvp("file", file), 
            cereal::make_nvp("comment", comment));
}


template void STI::Engine::ShotConfig::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ShotConfig::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
