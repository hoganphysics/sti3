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

	ParseID parseID;
	
	TimeStamp submissionTime;	//when the shot was submitted (not when it was played)
	TimeStamp playTime;

	EngineJobSourceID jobSourceID;

	// std::string user;
	// std::string machine;

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

private:

	static TimeStamp lastPlayTime;
	static std::mutex IDmutex;

};


} //Engine
} //STI

#endif

