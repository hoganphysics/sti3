
#include "ShotResult.h"
#include "ShotID.h"
#include "Measurement.h"
#include "RawEvent.h"
#include "utils/FileHolder.h"


#include "CerealArchives.h"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::ShotResult;


template<class Archive>
void ShotResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("ShotID", sid),
        cereal::make_nvp("Attributes", attributes), 
        cereal::make_nvp("Measurements", measurements),
        cereal::make_nvp("timingFiles", timingFiles),
        cereal::make_nvp("parsedEvents", parsedEvents)
        );
}


template void ShotResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ShotResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
