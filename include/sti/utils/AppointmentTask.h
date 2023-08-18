#ifndef STI_UTILS_APPOINTMENTTASK_H
#define STI_UTILS_APPOINTMENTTASK_H

#include <sti/utils/Task.h>

#include <chrono>
#include <functional>


namespace STI
{
namespace Utils
{


///A task that happens at a specific time
class AppointmentTask : public Task
{
public:

	AppointmentTask(int id, const std::string& timeOfDay, const std::function<void(void)>& runFunc);
	~AppointmentTask();

	double secondsToNextRun() const;
	void run();
	void skipTask();
	bool repeat();

private:

	std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> nextRunTime;
	std::chrono::seconds timeAfterMidnight;		//seconds after midnight (local time) when task occurs

	const std::function<void(void)> runFunc;
};


} // UTILS
} // STI


#endif
