
#include <sti/engine/EngineJobStatus.h>

using STI::Engine::EngineJobStatus;
// using STI::Engine::EngineJobStatusToString;


std::string STI::Engine::EngineJobStatusToString(const STI::Engine::EngineJobStatus& status)
{
    //{ New, Running, Completed, Canceled, NotFound, Archived, Deferred }
    std::string result = "";
    switch (status)
    {
    case EngineJobStatus::New:
        result = "New";
        break;
    case EngineJobStatus::Running:
        result = "Running";
        break;
    case EngineJobStatus::Completed:
        result = "Completed";
        break;
    case EngineJobStatus::Canceled:
        result = "Canceled";
        break;
    case EngineJobStatus::NotFound:
        result = "NotFound";
        break;
    case EngineJobStatus::Archived:
        result = "Archived";
        break;
    case EngineJobStatus::Deferred:
        result = "Deferred";
        break;    
    default:
        break;
    }
    return result;
}
