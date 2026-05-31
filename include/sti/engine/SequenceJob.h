#ifndef STI_ENGINE_SEQUENCEJOB_H
#define STI_ENGINE_SEQUENCEJOB_H

#include <sti/device/DeviceID.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/utils/TimeStamp.h>
// #include <sti/engine/Sequence.h>
// #include <sti/engine/SequenceResult.h>
// #include "utils/OrderedBufferMap.h"
#include <sti/utils/SynchronizedMap.h>

namespace STI
{
namespace Engine
{

class Sequence;
class SequenceResult;


class SequenceJob
{
public:

    SequenceJob(const EngineJobID& jobID, const STI::Device::DeviceID& jobOwner, 
        const std::shared_ptr<Sequence>& sequence, const std::shared_ptr<SequenceResult>& sequenceResult);
    virtual ~SequenceJob() = default;

    EngineJobID jobID;
    STI::Device::DeviceID jobOwner;
    
    EngineJobStatus getJobStatus();
    void close();
    void cancel();
    bool allShotsDone() const;
    bool isDone();
    bool hasPriority(const std::set<EngineJobID>& jobsIDs);
    void markJobSubmitted();
    bool hasSubmittedJobs() const;

    std::shared_ptr<Sequence> sequence;
    std::shared_ptr<SequenceResult> sequenceResult;

    STI::Utils::TimeStamp lastJobTimestamp;
    
    // STI::Utils::OrderedBufferMap<EngineJobID, std::shared_ptr<EventEngineJob>> runningJobs;
    STI::Utils::SynchronizedMap<EngineID, std::shared_ptr<EventEngineJob>> runningJobs;

    void filter(const std::set<EngineJobID>& queued, std::set<EngineJobID>& filtered);
    bool isMemberOfSequence(const EngineJobID& jobID);

private:

    EngineJobStatus jobStatus;
    bool jobSubmitted;

    void refreshJobStatus();
};


} //Engine
} //STI

#endif
