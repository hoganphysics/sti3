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
    prioritySet = true;
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
    std::vector<std::shared_ptr<EventEngineJob>> jobs;
    runningJobs.getValues(jobs);

    for (const auto& job : jobs) {
        if (job != 0 && job->getStatus() == EngineJobStatus::Running) {
            prioritySet = true;
            return true;
        }
    }

    if (prioritySet) {
        //has priority, for now
        prioritySet = false;
        STI::Utils::TimeStamp now;
        lastJobTimestamp = now;
        lastJobTimestamp.add_ms(200); //give it a 200 ms buffer
        return true;
    }

    bool found = false;
    //check if any of the jobs in the set is part of this sequence job
    for (const auto& jid : jobsIDs) {
        if (isMemberOfSequence(jid)) {
            prioritySet = true;
            return true;
        }
        else if (lastJobTimestamp < jid.runTime) {
            //if the job is newer than the last job timestamp+buffer, the sequence losses priority
            found = true;
        }
    }

    if (found) {
        return false;
    }

    return true; //no newer jobs found, so the sequence retains priority
}

bool SequenceJob::isMemberOfSequence(const EngineJobID& jid)
{
    return (jid.pid.sequenceEntryID.seqID == jobID.seqid ||
            jid.sid.parseID.sequenceEntryID.seqID == jobID.seqid);
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