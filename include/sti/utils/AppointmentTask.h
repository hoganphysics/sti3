#ifndef STI_UTILS_APPOINTMENTTASK_H
#define STI_UTILS_APPOINTMENTTASK_H

#include <sti/utils/Task.h>

#include <chrono>
#include <functional>
#include <memory>


namespace STI
{
namespace Utils
{

class LocalTimePoint;

///A task that happens at a specific time
class AppointmentTask : public Task
{
public:

	enum class AppointmentRepeatType { Once, Everyday, Weekdays };

	AppointmentTask(const std::string& id, const std::string& timeOfDay, const std::function<void(void)>& runFunc);
	AppointmentTask(const std::string& id, const std::string& timeOfDay, const AppointmentRepeatType& repeatType, const std::function<void(void)>& runFunc);
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

	//This is a temporary work around because of missing support for 
	//std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> in gcc.
	//Hide the date library type behind an opaque type:
	std::unique_ptr<LocalTimePoint> nextRunTimeHolder;
	//For gcc-13, std::chrono can be used directly:
	//std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> nextRunTime;	//works in msvs-17, not in gcc-11

	std::chrono::seconds timeAfterMidnight;		//seconds after midnight (local time) when task occurs

	const std::function<void(void)> runFunc;
};


} // UTILS
} // STI


#endif
