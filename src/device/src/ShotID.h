#ifndef STI_ENGINE_SHOTID_H
#define STI_ENGINE_SHOTID_H

#include "ParseID.h"
#include "TimeStamp.h"

#include <string>
#include <mutex>

namespace STI
{
namespace Engine
{


class ShotID
{
public:

	ShotID() {}
	ShotID(const ParseID& pid, const EngineJobSourceID& jobSourceID) : parseID(pid), jobSourceID(jobSourceID) {}

	ParseID parseID;
	EngineJobSourceID jobSourceID;
	
	TimeStamp submissionTime;	//when the shot was submitted (not when it was played)

	bool operator<(const ShotID& rhs) const 
	{
		if (parseID == rhs.parseID) {
			return submissionTime < rhs.submissionTime; 
		}
		else {
			return parseID < rhs.parseID; 
		}
	}

	bool operator==(const ShotID& rhs) const { return parseID == rhs.parseID && submissionTime == rhs.submissionTime; }
	bool operator!=(const ShotID& rhs) const { return !((*this) == rhs); }

	static ShotID generateUniqueID(const ParseID& pid);

	template<class Archive>
	void serialize(Archive& archive);

private:

	static TimeStamp lastSubmissionTime;
	static std::mutex IDmutex;

};


} //Engine
} //STI

#endif

