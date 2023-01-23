#ifndef STI_ENGINE_SEQUENCE_H
#define STI_ENGINE_SEQUENCE_H

#include <set>
#include <map>
#include <mutex>

namespace STI
{
namespace Engine
{

class ParsedVar;


class SequenceEntry
{
public:

    SequenceEntry();

    int index;
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
    void addEntry(int index, const std::set<ParsedVar>& overwritten);
    void append(const std::set<ParsedVar>& overwritten);

    std::map<unsigned, SequenceEntry> sequenceTable;
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
