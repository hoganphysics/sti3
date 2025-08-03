#ifndef STI_ENGINE_SEQUENCEID_H
#define STI_ENGINE_SEQUENCEID_H

#include <sti/utils/TimeStamp.h>
#include <sti/engine/EngineJobSourceID.h>

#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{

class SequenceID;

class SequenceIndex
{
public:

	SequenceIndex();
	explicit SequenceIndex(int index, int repeat);

    int index;
    int repeat;

	bool operator<(const SequenceIndex& rhs) const  
	{
		if (repeat == rhs.repeat) {
			return index < rhs.index;
		}
		return repeat < rhs.repeat;
	}

	std::string print() const;

	bool operator==(const SequenceIndex& rhs) const { return repeat == rhs.repeat && index == rhs.index; }
	bool operator!=(const SequenceIndex& rhs) const { return !((*this) == rhs); }

	template<class Archive>
	void serialize(Archive& archive);
};


class SequenceID
{
public:

	SequenceID();
	SequenceID(const STI::Utils::TimeStamp& timestamp, const EngineJobSourceID& jobSourceID);

	STI::Utils::TimeStamp timestamp;
    EngineJobSourceID jobSourceID;

	bool operator<(const SequenceID& rhs) const  { return timestamp < rhs.timestamp; }
	bool operator==(const SequenceID& rhs) const { return timestamp == rhs.timestamp; }
	bool operator!=(const SequenceID& rhs) const { return !((*this) == rhs); }

	static SequenceID generateUniqueID(const EngineJobSourceID& source);

	std::string print() const;

	template<class Archive>
	void serialize(Archive& archive);

private:

	static STI::Utils::TimeStamp lastSubmissionTime;
	static std::mutex IDmutex;
};


class SequenceEntryID
{
public:

	SequenceEntryID();
	SequenceEntryID(const SequenceID& seqID, const SequenceIndex& seqIndex);

    SequenceID seqID;
    SequenceIndex seqIndex;

	bool operator<(const SequenceEntryID& rhs) const  
	{
		if (seqID == rhs.seqID) {
			return seqIndex < rhs.seqIndex;
		}
		return seqID < rhs.seqID;
	}
	bool operator==(const SequenceEntryID& rhs) const { return seqID == rhs.seqID && seqIndex == rhs.seqIndex; }
	bool operator!=(const SequenceEntryID& rhs) const { return !((*this) == rhs); }

	template<class Archive>
	void serialize(Archive& archive);
};



} //Engine
} //STI

#endif


