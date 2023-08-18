
#include <sti/utils/IntervalTask.h>

#include <iostream>

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
	auto now = std::chrono::system_clock::now();

	auto wait = (lastRunTime + std::chrono::seconds(static_cast<int>(waitInterval))) - now;

	auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(wait);

	double wait_s = static_cast<double>(wait_ms.count()) / 1000;

	//std::cout << "secondsToNextRun: " << wait_s << std::endl;
	return wait_s;
}

void IntervalTask::run()
{
	auto tmpnow = std::chrono::system_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(tmpnow - lastRunTime);
	//std::cout << "Interval: " << static_cast<double>(delta.count()) / 1000 << std::endl;

	lastRunTime = std::chrono::system_clock::now();
	runFunc();
}

void IntervalTask::skipTask()
{
	lastRunTime = std::chrono::system_clock::now();
}

bool IntervalTask::repeat()
{
	return true;
}
