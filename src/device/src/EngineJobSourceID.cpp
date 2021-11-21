
#include "EngineJobSourceID.h"

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

#include <sstream>

using STI::Engine::EngineJobSourceID;


template<class Archive>
void EngineJobSourceID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("user", user), 
            cereal::make_nvp("machine", machine));
}

std::string EngineJobSourceID::print() const
{
    std::stringstream job;

    job << "[ " << user << "@" << machine << " ]";
    return job.str();
}

template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

