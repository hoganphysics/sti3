
#include <sti/engine/Sequence.h>

#include <sti/engine/SequenceID.h>
#include <sti/engine/ParsedVar.h>


#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
// #include <cereal/types/memory.hpp>
// #include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>


using STI::Engine::Sequence;
using STI::Engine::SequenceType;
using STI::Engine::SequenceEntry;


Sequence::Sequence()
{
}

Sequence::Sequence(const SequenceType& type)
: type(type)
{
}

template<class Archive>
void Sequence::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("sequenceTable", sequenceTable),
        cereal::make_nvp("repeats", repeats)
        );
}

SequenceEntry::SequenceEntry()
{
    index = -1;
}

template<class Archive>
void SequenceEntry::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("index", index),
        cereal::make_nvp("overwritten", overwritten)
        );
}


template void Sequence::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void Sequence::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );



