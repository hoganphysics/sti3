
#include <sti/engine/EngineJobSourceID.h>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

#include <sstream>

using STI::Engine::EngineJobSourceID;



std::string EngineJobSourceID::toString() const
{
    std::stringstream job;

    job << user << "@" << machine;
    return job.str();
}

EngineJobSourceID EngineJobSourceID::fromString(const std::string& jobSourceID)
{
    auto atPos = jobSourceID.find('@');
    if (atPos == std::string::npos) {
        throw std::invalid_argument("Invalid EngineJobSourceID string format");
    }

    std::string user = jobSourceID.substr(0, atPos);
    std::string machine = jobSourceID.substr(atPos + 1);

    return EngineJobSourceID(user, machine);
}

std::string EngineJobSourceID::print() const
{
    return toString();
}

template<class Archive>
void EngineJobSourceID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("user", user), 
            cereal::make_nvp("machine", machine));
}

template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::EngineJobSourceID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

