#include <sti/engine/ShotID.h>

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Engine::ShotID;
using STI::Engine::ParseID;
using STI::Utils::TimeStamp;


TimeStamp ShotID::lastSubmissionTime;
std::mutex ShotID::IDmutex;


ShotID ShotID::generateUniqueID(const ParseID& pid, const EngineJobSourceID& jobSourceID)
{
    std::unique_lock<std::mutex> IDlock(IDmutex);

    ShotID sid(pid, jobSourceID);

    if (sid.submissionTime <= lastSubmissionTime) {
        //Ensure monotonic ShotID generation if clock resolution stalls or moves backward.
        sid.submissionTime = lastSubmissionTime;
        sid.submissionTime.add_ns(1);
    }

    lastSubmissionTime = sid.submissionTime;

    return sid;
}

std::string ShotID::print() const
{
	std::stringstream sid;

    sid << "sid:" << jobSourceID.print() << "#" << submissionTime.time_hh_mm_ss_mmmuuunnn();

    return sid.str();
}


template<class Archive>
void ShotID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("parseID", parseID), 
            cereal::make_nvp("jobSourceID", jobSourceID),
            cereal::make_nvp("submissionTime", submissionTime)
            );
}

template void STI::Engine::ShotID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::ShotID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
