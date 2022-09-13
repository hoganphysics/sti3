#ifndef STI_ENGINE_PARSEID_H
#define STI_ENGINE_PARSEID_H

#include <sti/engine/TimeStamp.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/ShotConfig.h>

#include <string>


namespace STI
{
namespace Engine
{


class ParseID
{
public:

	ParseID();

	TimeStamp parseTimestamp;
	ShotConfig shotConfig;

	bool operator<(const ParseID& rhs) const { return parseTimestamp < rhs.parseTimestamp; }
	bool operator==(const ParseID& rhs) const { return parseTimestamp == rhs.parseTimestamp; }
	bool operator!=(const ParseID& rhs) const { return !((*this) == rhs); }

	std::string print() const;

	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

