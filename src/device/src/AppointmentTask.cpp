
#include <sti/utils/AppointmentTask.h>

#include <sstream>
#include <iostream>

using STI::Utils::AppointmentTask;


AppointmentTask::AppointmentTask(int id, const std::string& timeOfDay, const std::function<void(void)>& runFunc)
	: Task(id), runFunc(runFunc)
{
	auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

	//At midnight local time:
	auto today = std::chrono::floor<std::chrono::days>(now);
	auto tomorrow = today + std::chrono::days(1);

	std::stringstream timeSS;
	timeSS << timeOfDay;

	timeSS >> std::chrono::parse("%H:%M:%S", timeAfterMidnight);	//convert to seconds

	if (today + timeAfterMidnight > now) {
		nextRunTime = today + timeAfterMidnight;
	}
	else {
		nextRunTime = tomorrow + timeAfterMidnight;
	}

	//std::cout << "today: " << today << std::endl;
	//std::cout << "tomorrow: " << tomorrow << std::endl;
	//std::cout << "nextRunTime: " << nextRunTime << std::endl;
}

AppointmentTask::~AppointmentTask()
{
}

double AppointmentTask::secondsToNextRun() const
{
	auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto wait = nextRunTime - now;

	auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(wait);

	double wais_s = static_cast<double>(wait_ms.count()) / 1000;

	return wais_s;
}

void AppointmentTask::run()
{
	runFunc();
}

void AppointmentTask::skipTask()
{
}

bool AppointmentTask::repeat()
{
	return false;
}
