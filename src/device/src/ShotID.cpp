
#include "ShotID.h"

#include <chrono>

using STI::Engine::ShotID;
using STI::Engine::ParseID;
using STI::Engine::TimeStamp;

TimeStamp ShotID::lastPlayTime;
std::mutex ShotID::IDmutex;

ShotID ShotID::generateUniqueID(const ParseID& pid)
{
    std::unique_lock<std::mutex> IDlock(IDmutex);

    ShotID sid;

    sid.parseID = pid;


    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp);

    sid.submissionTime.timestamp = ms.count();

    if (sid.submissionTime.timestamp == lastPlayTime.timestamp) {
        //error: increment TimeStamp to ensure ShotID is unique!
    }

    lastPlayTime = sid.submissionTime;

    return sid;
}

