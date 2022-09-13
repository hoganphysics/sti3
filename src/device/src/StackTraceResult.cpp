#include <sti/engine/StackTraceResult.h>

#include "StackTraceData.h"

#include "CerealArchives.h"
#include <cereal/types/memory.hpp>

using STI::Engine::ParseID;
using STI::Engine::StackTraceResult;


StackTraceResult::StackTraceResult()
{
}

StackTraceResult::StackTraceResult(const ParseID& pid)
: pid(pid)
{
}

template<class Archive>
void StackTraceResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("pid", pid),
        cereal::make_nvp("stackTraceData", stackTraceData)
        );
}


template void StackTraceResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void StackTraceResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
