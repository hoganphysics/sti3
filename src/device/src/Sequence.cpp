
#include <sti/engine/Sequence.h>

#include <sti/engine/SequenceID.h>
#include <sti/engine/ParsedVar.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/set.hpp>

#include <sstream>

using STI::Engine::Sequence;
using STI::Engine::SequenceType;
using STI::Engine::SequenceEntry;
using STI::Engine::ParsedVar;


Sequence::Sequence()
: Sequence(SequenceType::Open)
{
}

Sequence::Sequence(const SequenceType& type)
: type(type)
{
}

void Sequence::addEntry(const SequenceEntry& entry)
{
    std::unique_lock<std::mutex> seqLock(sequenceMutex);
    sequenceTable[entry.index] = entry;
}

void Sequence::addEntry(const SequenceIndex& index, const std::set<ParsedVar>& overwritten)
{
    std::unique_lock<std::mutex> seqLock(sequenceMutex);

    sequenceTable[index].index = index;
    sequenceTable[index].overwritten = overwritten;
}

void Sequence::append(const std::set<ParsedVar>& overwritten)
{
    SequenceIndex index;

    {
        std::unique_lock<std::mutex> seqLock(sequenceMutex);
        
        auto it = sequenceTable.rbegin();   //last element
        
        if (it != sequenceTable.rend()) {
            index = it->first;
            index.index++;  //increment index for next entry
        }
        else {
            //map is empty
            index.index = 0;
            index.repeat = 0;  //default repeat
        }
    }

    addEntry(index, overwritten);
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
    index.index = -1;
    index.repeat = 0;
}

std::string SequenceEntry::print() const
{
    std::stringstream entry;
    // entry << "<index=" << index.index;
    // if (index.repeat > 0) {
    //     entry << ", repeat=" << index.repeat;
    // }
    // entry << ">";
    entry << "[";
    bool first = true;
    for (const auto& var : overwritten) {
        if (!first) {
            entry << ", ";
        }
        first = false;
        entry << var.name << "=";
        entry << var.value.print();
    }
    entry << "]";
    return entry.str();
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



