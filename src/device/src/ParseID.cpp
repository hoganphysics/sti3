#include <sti/engine/ParseID.h>
#include <sti/engine/EngineJobSourceID.h>

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Engine::ParseID;
using STI::Engine::EngineJobSourceID;
using STI::Utils::TimeStamp;

TimeStamp ParseID::lastSubmissionTime;
std::mutex ParseID::IDmutex;

ParseID::ParseID()
{
    shotType = ShotType::Single;
}

std::string ParseID::toString() const
{
    std::stringstream pid;

    pid << "pid:" << jobSourceID.toString() << "#" << parseTimestamp.toString();

    return pid.str();
}

ParseID ParseID::fromString(const std::string& pid)
{
    ParseID parseID;

    //pid:user@machine#2023/09/15|11:45:09.235.567.129

    size_t pos1 = pid.find(":");
    size_t pos2 = pid.find("#");

    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 <= pos1) {
        return ParseID();   //invalid
    }

    std::string jobSourceIDstr = pid.substr(pos1 + 1, pos2 - (pos1 + 1));
    std::string timeStampStr = pid.substr(pos2 + 1);

    parseID.jobSourceID = EngineJobSourceID::fromString(jobSourceIDstr);
    parseID.parseTimestamp = TimeStamp::fromString(timeStampStr);

    return parseID;
}

std::string ParseID::print() const
{
    return toString();
}

template<class Archive>
void ParseID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseTimestamp", parseTimestamp),
            cereal::make_nvp("shotType", shotType),
            cereal::make_nvp("jobSourceID", jobSourceID),
            cereal::make_nvp("sequenceEntryID", sequenceEntryID)
        );

}

ParseID ParseID::generateUniqueID(const EngineJobSourceID& jobSourceID)
{
    std::unique_lock<std::mutex> IDlock(IDmutex);

    TimeStamp currentTime;

    if (currentTime == lastSubmissionTime) {
        //error: increment TimeStamp to ensure ParseID is unique!
        currentTime.add_ns(1);
    }

    ParseID pid(currentTime, jobSourceID);
    pid.shotType = ShotType::Single;
    lastSubmissionTime = currentTime;

    return pid;
}

ParseID ParseID::generateUniqueID(const EngineJobSourceID& jobSourceID, const SequenceEntryID& sequenceEntryID)
{
    auto pid = generateUniqueID(jobSourceID);
    pid.shotType = ShotType::SequenceEntry;
    pid.sequenceEntryID = sequenceEntryID;
    return pid;
}


template void STI::Engine::ParseID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ParseID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

