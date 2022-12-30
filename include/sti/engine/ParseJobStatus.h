
#ifndef STI_ENGINE_PARSEJOBSTATUS_H
#define STI_ENGINE_PARSEJOBSTATUS_H

#include <sti/engine/ParseID.h>
#include <sti/engine/EngineJobStatus.h>


namespace STI
{
namespace Engine
{

class ParseJobStatus
{
public:

    EngineJobStatus status;
    ParseID pid;
};


} //Engine
} //STI

#endif
