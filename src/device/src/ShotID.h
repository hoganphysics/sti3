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
	TimeStamp shotTimeStamp;
	std::string user;
	std::string machine;

	bool operator<(const ShotID& rhs) const { return shotTimeStamp < rhs.shotTimeStamp; }
	bool operator==(const ShotID& rhs) const { return parseID == rhs.parseID && shotTimeStamp == rhs.shotTimeStamp; }
	bool operator!=(const ShotID& rhs) const { return !((*this) == rhs); }

};


} //Engine
} //STI

#endif

