#include <sti/engine/SequenceJob.h>

#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EventEngineJob.h>

using STI::Engine::SequenceJob;
using STI::Engine::Sequence;
using STI::Engine::SequenceResult;
using STI::Engine::EngineJobStatus;
using STI::Engine::EventEngineJob;

#include <vector>
#include <set>
#include <memory>


SequenceJob::SequenceJob(const EngineJobID& jobID, const STI::Device::DeviceID& jobOwner,
    const std::shared_ptr<Sequence>& sequence, const std::shared_ptr<SequenceResult>& sequenceResult)
: jobID(jobID), jobOwner(jobOwner), jobStatus(EngineJobStatus::New), sequence(sequence), sequenceResult(sequenceResult)
{
    jobSubmitted = false;
}

void SequenceJob::refreshJobStatus()
{
    if (sequenceResult != 0) {
        // jobStatus = EngineJobStatus::New;

        if (sequence->type == STI::Engine::SequenceType::Closed && 
            sequenceResult->status.size() == sequence->sequenceTable.size() && allShotsDone()) {
            //all shots are done
            jobStatus = EngineJobStatus::Completed;
        }
    }
    else {
        jobStatus = EngineJobStatus::NotFound;
    }
}

bool SequenceJob::allShotsDone() const
{
    for (const auto& entry : sequence->sequenceTable) {
        if (sequenceResult->status.count(entry.first) == 0 || 
            sequenceResult->status.at(entry.first) == EngineJobStatus::New || 
            sequenceResult->status.at(entry.first) == EngineJobStatus::Running ||
            sequenceResult->status.at(entry.first) == EngineJobStatus::Deferred) {
            return false; //not all shots are done
        }
    }
    return true; //all shots are done
}

EngineJobStatus SequenceJob::getJobStatus()
{
    refreshJobStatus();
    return jobStatus;
}

bool SequenceJob::isDone() 
{
    refreshJobStatus();

    return jobStatus != EngineJobStatus::New && 
           jobStatus != EngineJobStatus::Running && 
           jobStatus != EngineJobStatus::Deferred;
}

void SequenceJob::close()
{
    if (sequence != 0) {
        if (sequence->type == STI::Engine::SequenceType::Open) {
            jobStatus = EngineJobStatus::Completed;
        }
        sequence->type = STI::Engine::SequenceType::Closed;
    }
    refreshJobStatus();
}

void SequenceJob::cancel()
{
    if (sequence != 0) {
        sequence->type = STI::Engine::SequenceType::Closed; //close the sequence
    }

    for (auto& entry : sequence->sequenceTable) {
        
        auto it = sequenceResult->status.find(entry.first);
        
        if (it != sequenceResult->status.end()) {
            //if the shot is not completed, set it to canceled

            if (it->second == EngineJobStatus::New || 
                it->second == EngineJobStatus::Running ||
                it->second == EngineJobStatus::Deferred) {

                sequenceResult->status[entry.first] = EngineJobStatus::Canceled;
            }
        }
        else {
            //not found; add a canceled status
            sequenceResult->status[entry.first] = EngineJobStatus::Canceled;
        }
    }
}

bool SequenceJob::hasPriority(const std::set<EngineJobID>& jobsIDs)
{
    (void)jobsIDs;
    return hasSubmittedJobs();
}

bool SequenceJob::isMemberOfSequence(const EngineJobID& jid)
{
    if (jid.type == EventEngineJobType::Parse && jid.pid.shotType == ShotType::SequenceEntry) {
        return jid.pid.sequenceEntryID.seqID == jobID.seqid;
    }

    if (jid.type == EventEngineJobType::Play && jid.sid.parseID.shotType == ShotType::SequenceEntry) {
        return jid.sid.parseID.sequenceEntryID.seqID == jobID.seqid;
    }

    return false;
}

void SequenceJob::markJobSubmitted()
{
    jobSubmitted = true;
}

bool SequenceJob::hasSubmittedJobs() const
{
    return jobSubmitted;
}

void SequenceJob::filter(const std::set<EngineJobID>& queued, std::set<EngineJobID>& filtered)
{
    filtered.clear();

    for (const auto& jid : queued) {
        if (isMemberOfSequence(jid))
        {
            filtered.insert(jid);
        }
    }

}
