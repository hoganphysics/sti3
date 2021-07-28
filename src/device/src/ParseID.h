#ifndef STI_ENGINE_PARSEID_H
#define STI_ENGINE_PARSEID_H

#include "TimeStamp.h"

#include <string>


namespace STI
{
namespace Engine
{

class EngineJobSourceID
{
public:
	std::string user;
	std::string machine;

	template<class Archive>
	void serialize(Archive& archive);
};


class ParseID
{
public:

	TimeStamp parseTimestamp;
	std::string file;	//primary file

	EngineJobSourceID jobSourceID;

	std::string comment;	//optional description of this shot

	bool operator<(const ParseID& rhs) const { return parseTimestamp < rhs.parseTimestamp; }
	bool operator==(const ParseID& rhs) const { return parseTimestamp == rhs.parseTimestamp && file.compare(rhs.file) == 0; }
	bool operator!=(const ParseID& rhs) const { return !((*this) == rhs); }

	enum class ShotType { Single, Sequence };//?

	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

