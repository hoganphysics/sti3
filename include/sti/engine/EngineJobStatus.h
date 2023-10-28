#ifndef STI_ENGINE_ENGINEJOBSTATUS_H
#define STI_ENGINE_ENGINEJOBSTATUS_H

#include <string>

namespace STI
{
namespace Engine
{

enum class EngineJobStatus { New, Running, Completed, Canceled, NotFound, Archived, Deferred };

std::string EngineJobStatusToString(const STI::Engine::EngineJobStatus& status);


} //Engine
} //STI

#endif
