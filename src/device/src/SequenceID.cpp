
#include <sti/engine/SequenceID.h>

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Engine::SequenceID;
using STI::Utils::TimeStamp;
using STI::Engine::SequenceIndex;
using STI::Engine::SequenceEntryID;


TimeStamp SequenceID::lastSubmissionTime;
std::mutex SequenceID::IDmutex;

SequenceID::SequenceID()
{
}

SequenceID SequenceID::generateUniqueID(const EngineJobSourceID& source)
{
    std::unique_lock<std::mutex> IDlock(IDmutex);

    SequenceID seqId;

    seqId.jobSourceID = source;

    if (seqId.timestamp == lastSubmissionTime) {
        //error: increment TimeStamp to ensure ShotID is unique!
        seqId.timestamp.add_ns(1);
    }

    lastSubmissionTime = seqId.timestamp;

    return seqId;
}

std::string SequenceID::print() const
{
	std::stringstream seqid;

    // seqid:user@machine#timestamp

    seqid << "seqid:" << jobSourceID.print() << "#" << timestamp.time_hh_mm_ss_mmmuuunnn();

    return seqid.str();
}


template<class Archive>
void SequenceID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("timestamp", timestamp), 
            cereal::make_nvp("jobSourceID", jobSourceID)
            );
}

SequenceIndex::SequenceIndex()
:SequenceIndex(-1, 0)
{
}

SequenceIndex::SequenceIndex(int index, int repeat)
: index(index), repeat(repeat)
{
}

std::string SequenceIndex::print() const
{
    std::stringstream seqIndex;

    seqIndex << "<index=" << index
             << ", repeat=" << repeat << ">";

    return seqIndex.str();
}

template<class Archive>
void SequenceIndex::serialize(Archive& archive)
{
    archive(cereal::make_nvp("index", index), 
            cereal::make_nvp("repeat", repeat)
            );
}

SequenceEntryID::SequenceEntryID()
{
}

template<class Archive>
void SequenceEntryID::serialize(Archive& archive)
{
    archive(cereal::make_nvp("SequenceID", seqID), 
            cereal::make_nvp("SequenceIndex", seqIndex)
            );
}


template void STI::Engine::SequenceID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::SequenceID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void STI::Engine::SequenceEntryID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::SequenceEntryID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void STI::Engine::SequenceIndex::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void STI::Engine::SequenceIndex::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
