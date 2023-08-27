
#include <sti/utils/IntervalTask.h>

//#include <iostream>

#include <sstream>
// #include <iostream>

#ifdef USE_DATE_LIB
#include <sti/extern/date/date.h>
#include <sti/extern/date/tz.h>
using namespace date;
#else
using namespace std::chrono;
#endif


using STI::Utils::IntervalTask;


IntervalTask::IntervalTask(const std::string& id, double wait_seconds, const std::function<void(void)>& runFunc)
: Task(id), waitInterval(wait_seconds), runFunc(runFunc)
{
	lastRunTime = std::chrono::system_clock::now();
	if (waitInterval < 1) {
		waitInterval = 1;	//minimum wait 1 second
	}
}

IntervalTask::IntervalTask(const std::string& id, const std::string& wait_time, const std::function<void(void)>& runFunc)
: Task(id), runFunc(runFunc)
{
	std::chrono::seconds wait_time_s;

	std::stringstream timeSS;
	timeSS << wait_time;
	timeSS >> parse("%H:%M:%S", wait_time_s);	//convert to seconds

	// auto tp_seconds = floor<std::chrono::seconds>(wait_time_s);
	// std::cout << "tp_seconds: " << tp_seconds << std::endl;

	waitInterval = static_cast<double>(wait_time_s.count());

	// std::cout << "waitInterval: " << waitInterval << " : " << static_cast<int>(waitInterval) << std::endl;

	lastRunTime = std::chrono::system_clock::now();
	if (waitInterval < 1) {
		waitInterval = 1;	//minimum wait 1 second
	}
}

IntervalTask::~IntervalTask()
{
}

double IntervalTask::secondsToNextRun() const
{
	using namespace std::chrono;

	auto now = std::chrono::system_clock::now();

	auto wait = (lastRunTime + seconds( static_cast<int>(waitInterval) )) - now;

	auto wait_ms = duration_cast<milliseconds>(wait);

	double wait_s = static_cast<double>(wait_ms.count()) / 1000;

	// std::cout << "secondsToNextRun: " << wait_s << " : " << wait << std::endl;
	return wait_s;
}

void IntervalTask::run()
{
	runFunc();

	lastRunTime = std::chrono::system_clock::now();
}

void IntervalTask::skipTask()
{
	lastRunTime = std::chrono::system_clock::now();
}

bool IntervalTask::repeat()
{
	return true;
}
