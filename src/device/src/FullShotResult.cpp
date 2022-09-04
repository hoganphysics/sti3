
#include <sti/engine/FullShotResult.h>

#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotResult.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

using STI::Engine::FullShotResult;


template<class Archive>
void FullShotResult::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("parseResult", parseResult), 
		cereal::make_nvp("shotResult", shotResult)
		);
}


template void FullShotResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void FullShotResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
