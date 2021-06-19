
#include "ShotID.h"

using STI::Engine::ShotID;
using STI::Engine::ParseID;
using STI::Engine::TimeStamp;

TimeStamp ShotID::lastSubmissionTime;
std::mutex ShotID::IDmutex;


ShotID ShotID::generateUniqueID(const ParseID& pid)
{
    std::unique_lock<std::mutex> IDlock(IDmutex);

    ShotID sid;

    sid.parseID = pid;

    if (sid.submissionTime == lastSubmissionTime) {
        //error: increment TimeStamp to ensure ShotID is unique!
        sid.submissionTime.add_ns(1);
    }

    lastSubmissionTime = sid.submissionTime;

    return sid;
}

