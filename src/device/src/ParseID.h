#ifndef STI_ENGINE_PARSEID_H
#define STI_ENGINE_PARSEID_H

#include "TimeStamp.h"
#include "EngineJobSourceID.h"
#include "ShotConfig.h"

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

	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

