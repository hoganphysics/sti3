#ifndef STI_ENGINE_PARSEID_H
#define STI_ENGINE_PARSEID_H

#include "TimeStamp.h"

#include <string>

namespace STI
{
namespace Engine
{

class ParseID
{
public:
	TimeStamp timestamp;
	std::string file;
	std::string user;
	std::string machine;

	bool operator==(const ParseID& rhs) const { return timestamp == rhs.timestamp && file.compare(rhs.file) == 0; }
	bool operator!=(const ParseID& rhs) const { return !((*this) == rhs); }

};


} //Engine
} //STI

#endif

