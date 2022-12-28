#ifndef STI_ENGINE_SEQUENCEID_H
#define STI_ENGINE_SEQUENCEID_H

#include <sti/engine/TimeStamp.h>
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

    int index;
    int repeat;

	bool operator<(const SequenceIndex& rhs) const  
	{
		if (repeat == rhs.repeat) {
			return index < rhs.index;
		}
		return repeat < rhs.repeat;
	}
	
	bool operator==(const SequenceIndex& rhs) const { return repeat == rhs.repeat && index == rhs.index; }
	bool operator!=(const SequenceIndex& rhs) const { return !((*this) == rhs); }

	template<class Archive>
	void serialize(Archive& archive);
};


class SequenceID
{
public:

	SequenceID();

	TimeStamp timestamp;
    EngineJobSourceID jobSourceID;

	bool operator<(const SequenceID& rhs) const  { return timestamp < rhs.timestamp; }
	bool operator==(const SequenceID& rhs) const { return timestamp == rhs.timestamp; }
	bool operator!=(const SequenceID& rhs) const { return !((*this) == rhs); }

	static SequenceID generateUniqueID(const EngineJobSourceID& source);

	std::string print() const;

	template<class Archive>
	void serialize(Archive& archive);

private:

	static TimeStamp lastSubmissionTime;
	static std::mutex IDmutex;
};


class SequenceEntryID
{
public:

	SequenceEntryID();

    SequenceID seqID;
    SequenceIndex seqIndex;

	template<class Archive>
	void serialize(Archive& archive);
};



} //Engine
} //STI

#endif


