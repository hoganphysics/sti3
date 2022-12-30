
#ifndef STI_ENGINE_PLAYJOBSTATUS_H
#define STI_ENGINE_PLAYJOBSTATUS_H

#include <sti/engine/ShotID.h>
#include <sti/engine/EngineJobStatus.h>


namespace STI
{
namespace Engine
{

class PlayJobStatus
{
public:

    EngineJobStatus status;
    ShotID sid;
};


} //Engine
} //STI

#endif
