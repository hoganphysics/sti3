#include <sti/engine/ParseID.h>
#include <sti/engine/EngineJobSourceID.h>

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Engine::ParseID;
using STI::Engine::EngineJobSourceID;


ParseID::ParseID()
{
}

std::string ParseID::print() const
{
	std::stringstream pid;

    pid << "pid:" << shotConfig.jobSourceID.print() << "#" << parseTimestamp.time_hh_mm_ss_mmmuuunnn();

    return pid.str();
}

template<class Archive>
void ParseID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseTimestamp", parseTimestamp),
            cereal::make_nvp("ShotConfig", shotConfig),
            cereal::make_nvp("sequenceEntryID", sequenceEntryID)
        );

}

template void STI::Engine::ParseID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ParseID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

