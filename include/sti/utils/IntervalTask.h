#ifndef STI_UTILS_INTERVALTASK_H
#define STI_UTILS_INTERVALTASK_H

#include <sti/utils/Task.h>

#include <chrono>
#include <functional>


namespace STI
{
namespace Utils
{


///Tasks that happen repeatedly after a fixed time interval
class IntervalTask : public Task
{
public:

	IntervalTask(int id, double wait_seconds, const std::function<void(void)>& runFunc);
	~IntervalTask();

	double secondsToNextRun() const;

	void run();
	void skipTask();
	bool repeat();

private:

	double waitInterval;
	const std::function<void(void)> runFunc;

	std::chrono::system_clock::time_point lastRunTime;
};


} // UTILS
} // STI


#endif
