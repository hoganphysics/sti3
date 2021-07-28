
#include "ShotID.h"

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

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

template<class Archive>
void ShotID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseID", parseID), 
            cereal::make_nvp("submissionTime", submissionTime), 
            cereal::make_nvp("playTime", playTime), 
            cereal::make_nvp("jobSourceID", jobSourceID));
}

template void STI::Engine::ShotID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ShotID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
