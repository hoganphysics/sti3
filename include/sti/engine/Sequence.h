#ifndef STI_ENGINE_SEQUENCE_H
#define STI_ENGINE_SEQUENCE_H

#include <set>
#include <map>


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

    template<class Archive>
	void serialize(Archive& archive);
};


enum class SequenceType { Open, Closed };

class Sequence
{
public:

    Sequence();
    Sequence(const SequenceType& type);

    std::map<unsigned, SequenceEntry> sequenceTable;
    unsigned repeats;
    SequenceType type;

    // bool next(SequenceEntry& entry);

    template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
