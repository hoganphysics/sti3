
#include "ShotConfig.h"

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Engine::ShotConfig;
using STI::Engine::ShotType;


ShotConfig::ShotConfig()
{
    targetEnginePool = 1;   //1=common shot pool, 0=async pool (readChannel/writeChannel)
    shotType = ShotType::Single;
}


std::string ShotConfig::print() const
{
	std::stringstream config;

    config << "<Type=" << printShotType(shotType);
    config << ", Source=" << jobSourceID.print();
    config << ", Pool=" << targetEnginePool;
    config << ", File=" << file;
    config << ">";

    return config.str();
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

std::string STI::Engine::printShotType(const ShotType& type)
{
    std::string result;

    switch (type)
    {
    case ShotType::Single:
        result = "Single";
        break;
    case ShotType::Sequence:
        result = "Sequence";
        break;
    case ShotType::SingleUndocumented:
        result = "SingleUndocumented";
        break;
    default:
        result = "Unknown";
        break;
    }
    return result;
}


template void STI::Engine::ShotConfig::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ShotConfig::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
