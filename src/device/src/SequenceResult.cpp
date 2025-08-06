
#include <sti/engine/SequenceResult.h>

#include <sti/engine/ParsedVar.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/ShotID.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::SequenceResult;
using STI::Engine::Sequence;
using STI::Engine::SequenceID;
using STI::Engine::SequenceIndex;
using STI::Engine::ShotID;


SequenceResult::SequenceResult()
{
}

SequenceResult::SequenceResult(const SequenceID& id, const std::shared_ptr<Sequence>& sequence)
: seqid(id), sequence(sequence)
{
    if (sequence == 0) {
        this->sequence = std::make_shared<Sequence>();
    }
}

bool SequenceResult::addShotResult(const SequenceIndex& index, const ShotID& shotID, const EngineJobStatus& shotStatus)
{
    // if (status.count(index) != 0) return false;
    // if (shots.count(index) != 0) return false;

    if (sequence->type == STI::Engine::SequenceType::Closed && 
        sequence->sequenceTable.count(index) == 0) {
        //sequence entry not found
        return false;
    }
    
    status[index] = shotStatus;
    shots[index] = shotID;

    return true;
}

SequenceIndex SequenceResult::append(const EngineJobStatus& shotStatus)
{
    SequenceIndex seqIndex;
    // index.index = sequence->sequenceTable.size();
    // index.repeat = sequence->repeats;

    // if (sequence->type == STI::Engine::SequenceType::Closed) {
    //     //append to closed sequence
    //     sequence->addEntry(index, std::set<STI::Engine::ParsedVar>());
    // }
    // else {
    //     //open sequence
    //     sequence->append(std::set<STI::Engine::ParsedVar>());
    // }
    // index.index = status.size();
    unsigned lastIndex = status.rbegin()->first.index;
    seqIndex.index = lastIndex + 1;
    seqIndex.repeat = 0; // default repeat

    status[seqIndex] = shotStatus;

    return seqIndex;
}

template<class Archive>
void SequenceResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("SequenceID", seqid),
        cereal::make_nvp("Sequence", sequence),
        cereal::make_nvp("status", status)
        );
}


template void SequenceResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void SequenceResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
