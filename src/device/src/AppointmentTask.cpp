
#include <sti/utils/AppointmentTask.h>

#include <sstream>
//#include <iostream>

#ifdef USE_DATE_LIB
#include <sti/extern/date/date.h>
#include <sti/extern/date/tz.h>
using namespace date;
#else
using namespace std::chrono;
#endif

using STI::Utils::AppointmentTask;

//Temporary hack due to lack of date support in gcc
namespace STI
{
namespace Utils
{
struct LocalTimePoint
{
	#ifdef USE_DATE_LIB
	std::chrono::time_point<date::local_t, std::chrono::seconds> nextRunTime;
	#else
	std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> nextRunTime;
	#endif
};
}
}

AppointmentTask::AppointmentTask(const std::string& id, const std::string& timeOfDay, const std::function<void(void)>& runFunc)
: AppointmentTask(id, timeOfDay, AppointmentTask::AppointmentRepeatType::Once, runFunc)
{
}


AppointmentTask::AppointmentTask(const std::string& id, const std::string& timeOfDay, const AppointmentTask::AppointmentRepeatType& repeatType,
	const std::function<void(void)>& runFunc)
: Task(id), runFunc(runFunc), timeOfDay(timeOfDay), repeatType(repeatType)
{
	nextRunTimeHolder = std::make_unique<STI::Utils::LocalTimePoint>();

	std::stringstream timeSS;
	timeSS << timeOfDay;
	timeSS >> parse("%H:%M:%S", timeAfterMidnight);	//convert to seconds

	computeNextRuntime();
	//std::cout << "nextRunTime: " << nextRunTime << std::endl;
}

AppointmentTask::~AppointmentTask()
{
}

void AppointmentTask::computeNextRuntime()
{
	switch (repeatType)
	{
	case AppointmentTask::AppointmentRepeatType::Once:
		computeNextRuntimeAnyday();
		break;
	case AppointmentTask::AppointmentRepeatType::Everyday:
		computeNextRuntimeAnyday();
		break;
	case AppointmentTask::AppointmentRepeatType::Weekdays:
		computeNextRuntimeWeekdays();
		break;
	default:
		break;
	}
}

void AppointmentTask::computeNextRuntimeAnyday()
{
	//using namespace std::chrono;

	auto now = current_zone()->to_local(std::chrono::system_clock::now());
	auto today = floor<days>(now);		//At midnight local time
	auto tomorrow = today + days(1);

	if (today + timeAfterMidnight > now) {
		nextRunTimeHolder->nextRunTime = today + timeAfterMidnight;
	}
	else {
		nextRunTimeHolder->nextRunTime = tomorrow + timeAfterMidnight;
	}

	//std::cout << "today: " << today << std::endl;
	//std::cout << "tomorrow: " << tomorrow << std::endl;
	//std::cout << "nextRunTime: " << nextRunTime << std::endl;
}

void AppointmentTask::computeNextRuntimeWeekdays()
{
	//using namespace std::chrono;

	auto now = current_zone()->to_local(std::chrono::system_clock::now());
	auto today = floor<days>(now);		//At midnight local time

	bool runToday;

	if (today + timeAfterMidnight > now) {
		//The appointment time hasn't happened yet today
		runToday = true;	//attempt to run today, if it's a weekday
	}
	else {
		runToday = false;	//run on next weekday
	}

	std::chrono::time_point<local_t, std::chrono::seconds> nextWeekday;

	weekday wd{ today };
	if (wd == Saturday) {
		nextWeekday = today + days(2);	//Monday
	}
	else if (wd == Sunday) {
		nextWeekday = today + days(1);	//Monday
	}
	else if (wd == Friday) {
		if (runToday) {
			nextWeekday = today;	//Friday
		}
		else {
			nextWeekday = today + days(3);	//Monday
		}
	}
	else {
		//M,T,W,Th
		if (runToday) {
			nextWeekday = today;
		}
		else {
			nextWeekday = today + days(1);	//Tomorrow
		}
	}

	nextRunTimeHolder->nextRunTime = nextWeekday + timeAfterMidnight;
}

double AppointmentTask::secondsToNextRun() const
{
	using namespace std::chrono;

	auto now = current_zone()->to_local(std::chrono::system_clock::now());

	auto wait = nextRunTimeHolder->nextRunTime - now;

	auto wait_ms = duration_cast<milliseconds>(wait);

	double wais_s = static_cast<double>(wait_ms.count()) / 1000;

	return wais_s;
}

void AppointmentTask::run()
{
	runFunc();
	computeNextRuntime();
}

void AppointmentTask::skipTask()
{
	computeNextRuntime();
}

bool AppointmentTask::repeat()
{
	bool taskRepeats = false;

	switch (repeatType)
	{
	case AppointmentTask::AppointmentRepeatType::Once:
		taskRepeats = false;
		break;
	case AppointmentTask::AppointmentRepeatType::Everyday:
		taskRepeats = true;
		break;
	case AppointmentTask::AppointmentRepeatType::Weekdays:
		taskRepeats = true;
		break;
	default:
		break;
	}

	return taskRepeats;
}
