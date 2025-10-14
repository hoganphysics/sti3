#ifndef STI_ENGINE_SEQUENCE_H
#define STI_ENGINE_SEQUENCE_H

#include <sti/engine/SequenceID.h>
#include <sti/engine/ShotConfig.h>

#include <set>
#include <map>
#include <mutex>
#include <string>

namespace STI
{
namespace Engine
{

class ParsedVar;


class SequenceEntry
{
public:

    SequenceEntry();

    SequenceIndex index;
    std::set<ParsedVar> overwritten;

	bool operator<(const SequenceEntry& rhs) const  { return index < rhs.index; }
	bool operator==(const SequenceEntry& rhs) const { return index == rhs.index; }
	bool operator!=(const SequenceEntry& rhs) const { return !((*this) == rhs); }

    std::string print() const;

    template<class Archive>
	void serialize(Archive& archive);
};


enum class SequenceType { Open, Closed };

class Sequence
{
public:

    Sequence();
    Sequence(const SequenceType& type);

    void addEntry(const SequenceEntry& entry);
    void addEntry(const SequenceIndex& index, const std::set<ParsedVar>& overwritten);
    void removeEntry(const SequenceIndex& index);

    void append(const std::set<ParsedVar>& overwritten);
    
    ShotConfig shotConfig;  //shot config for this sequence

    std::map<SequenceIndex, SequenceEntry> sequenceTable;
    unsigned repeats;
    SequenceType type;

    // bool next(SequenceEntry& entry);

    template<class Archive>
	void serialize(Archive& archive);

private:

    mutable std::mutex sequenceMutex;
};


} //Engine
} //STI

#endif
