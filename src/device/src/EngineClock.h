#ifndef STI_ENGINE_ENGINECLOCK_H
#define STI_ENGINE_ENGINECLOCK_H

#include <chrono>


namespace STI
{
namespace Engine
{


class EngineClock
{
public:

	EngineClock();

	void reset();
	void reset(double offset);

	int64_t getTime() const;

	int64_t getWaitInterval(double time) const;


private:

	std::chrono::high_resolution_clock::time_point tStart;

};


} //Engine
} //STI

#endif
