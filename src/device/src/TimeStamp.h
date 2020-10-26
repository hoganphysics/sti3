#ifndef STI_ENGINE_TIMESTAMP_H
#define STI_ENGINE_TIMESTAMP_H

#include <string>

namespace STI
{
namespace Engine
{

class TimeStamp
{
public:
	//date
	//time
	//timezone
	double timestamp;	//show use std::chrono
	std::string print();

	bool operator<(const TimeStamp& rhs) const { return timestamp < rhs.timestamp; }
	bool operator==(const TimeStamp& rhs) const { return timestamp == rhs.timestamp; }
	bool operator!=(const TimeStamp& rhs) const { return !((*this) == rhs); }
};


} //Engine
} //STI

#endif
