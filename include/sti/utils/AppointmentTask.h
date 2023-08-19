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

	enum class AppointmentRepeatType { Once, Everyday, Weekdays };

	AppointmentTask(int id, const std::string& timeOfDay, const std::function<void(void)>& runFunc);
	AppointmentTask(int id, const std::string& timeOfDay, const AppointmentRepeatType& repeatType, const std::function<void(void)>& runFunc);
	~AppointmentTask();

	double secondsToNextRun() const;
	void run();
	void skipTask();
	bool repeat();

private:

	void computeNextRuntime();
	void computeNextRuntimeAnyday();
	void computeNextRuntimeWeekdays();

	std::string timeOfDay;				//scheduled appointment time
	AppointmentRepeatType repeatType;

	std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> nextRunTime;
	std::chrono::seconds timeAfterMidnight;		//seconds after midnight (local time) when task occurs

	const std::function<void(void)> runFunc;
};


} // UTILS
} // STI


#endif
