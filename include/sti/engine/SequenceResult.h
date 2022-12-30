#ifndef STI_ENGINE_SEQUENCERESULT_H
#define STI_ENGINE_SEQUENCERESULT_H

#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/EngineJobStatus.h>


#include <memory>
#include <map>


namespace STI
{
namespace Engine
{

class ShotID;


class SequenceResult
{
public:

    SequenceResult();
    SequenceResult(const SequenceID& id, const std::shared_ptr<Sequence>& sequence);

    SequenceID seqid;
    std::shared_ptr<Sequence> sequence;

    std::map<SequenceIndex, EngineJobStatus> status;
    std::map<SequenceIndex, ShotID> shots;

    bool addShotResult(const SequenceIndex& index, const ShotID& shotID, const EngineJobStatus& shotStatus);

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

