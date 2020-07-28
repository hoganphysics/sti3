#ifndef STI_ENGINE_TIMESTAMP_H
#define STI_ENGINE_TIMESTAMP_H

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
	double timestamp;
	std::string print();

	bool operator<(const TimeStamp& rhs) const { return timestamp < rhs.timestamp; }
	bool operator==(const TimeStamp& rhs) const { return timestamp == rhs.timestamp; }
	bool operator!=(const TimeStamp& rhs) const { return !((*this) == rhs); }
};


} //Engine
} //STI

#endif
