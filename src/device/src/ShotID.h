#ifndef STI_ENGINE_SHOTID_H
#define STI_ENGINE_SHOTID_H

#include "ParseID.h"
#include "TimeStamp.h"

#include <string>

namespace STI
{
namespace Engine
{

class ShotID
{
public:

	ParseID parseID;
	
	TimeStamp submissionTime;	//when the shot was submitted (not when it was played)

	std::string user;
	std::string machine;

	bool operator<(const ShotID& rhs) const { return submissionTime < rhs.submissionTime; }
	bool operator==(const ShotID& rhs) const { return parseID == rhs.parseID && submissionTime == rhs.submissionTime; }
	bool operator!=(const ShotID& rhs) const { return !((*this) == rhs); }

};


} //Engine
} //STI

#endif

