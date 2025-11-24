#ifndef STI_ENGINE_PARSEID_H
#define STI_ENGINE_PARSEID_H

#include <sti/utils/TimeStamp.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/ShotConfig.h>

#include <string>
#include <mutex>

namespace STI
{
namespace Engine
{


class ParseID
{
public:

	ParseID();
	ParseID(const STI::Utils::TimeStamp& parseTimestamp, const EngineJobSourceID& jobSourceID)
	: parseTimestamp(parseTimestamp), jobSourceID(jobSourceID)  { shotType = ShotType::Single; }
	ParseID(const STI::Utils::TimeStamp& parseTimestamp, const EngineJobSourceID& jobSourceID, const SequenceEntryID& sequenceEntryID)
	: parseTimestamp(parseTimestamp), jobSourceID(jobSourceID), sequenceEntryID(sequenceEntryID) { shotType = ShotType::SequenceEntry; }

	STI::Utils::TimeStamp parseTimestamp;
	ShotType shotType;
	EngineJobSourceID jobSourceID;

	SequenceEntryID sequenceEntryID;

	bool operator<(const ParseID& rhs) const { return parseTimestamp < rhs.parseTimestamp; }
	bool operator==(const ParseID& rhs) const { return parseTimestamp == rhs.parseTimestamp; }
	bool operator!=(const ParseID& rhs) const { return !((*this) == rhs); }

	static ParseID generateUniqueID(const EngineJobSourceID& jobSourceID);
	static ParseID generateUniqueID(const EngineJobSourceID& jobSourceID, const SequenceEntryID& sequenceEntryID);

	std::string print() const;

	std::string toString() const;
	static ParseID fromString(const std::string& pid);

	template<class Archive>
	void serialize(Archive& archive);

private:

	static STI::Utils::TimeStamp lastSubmissionTime;
	static std::mutex IDmutex;
};


} //Engine
} //STI

#endif

