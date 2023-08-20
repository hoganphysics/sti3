
#include <sti/utils/IntervalTask.h>

//#include <iostream>

using STI::Utils::IntervalTask;


IntervalTask::IntervalTask(int id, double wait_seconds, const std::function<void(void)>& runFunc)
	: Task(id), waitInterval(wait_seconds), runFunc(runFunc)
{
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

	//std::cout << "secondsToNextRun: " << wait_s << std::endl;
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
