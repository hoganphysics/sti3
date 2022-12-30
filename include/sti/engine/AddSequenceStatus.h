
#ifndef STI_ENGINE_ADDSEQUENCESTATUS_H
#define STI_ENGINE_ADDSEQUENCESTATUS_H

#include <sti/engine/SequenceID.h>
#include <sti/engine/EngineJobStatus.h>


namespace STI
{
namespace Engine
{

class AddSequenceStatus
{
public:

    EngineJobStatus status;
    SequenceID seqid;
};

} //Engine
} //STI

#endif
