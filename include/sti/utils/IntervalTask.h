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

	IntervalTask(const std::string& id, double wait_seconds, const std::function<void(void)>& runFunc);
	IntervalTask(const std::string& id, const std::string& wait_time, const std::function<void(void)>& runFunc);	//format: hh:mm:ss
	~IntervalTask();

	double secondsToNextRun() const override;

	void skipTask() override;
	bool repeat() override;

private:

	void run() override;

	double waitInterval;
	const std::function<void(void)> runFunc;

	std::chrono::system_clock::time_point lastRunTime;
};


} // UTILS
} // STI


#endif
