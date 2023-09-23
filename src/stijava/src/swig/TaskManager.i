%{
    #include "JTaskManager.h"
    using STI::Device::JTaskManager;

    #include <sti/utils/Task.h>
    using STI::Utils::Task;
    using STI::Utils::TaskStatus;

    #include <sti/utils/IntervalTask.h>
    using STI::Utils::IntervalTask;
    #include <sti/utils/AppointmentTask.h>
    using STI::Utils::AppointmentTask;
    // using STI::Utils::AppointmentTask::AppointmentRepeatType;

    #include "RunnableTask.h"
    using STI::Device::RunnableTask;

%}

%shared_ptr(STI::Utils::Task);
%shared_ptr(STI::Utils::IntervalTask);
%shared_ptr(STI::Utils::AppointmentTask);
%shared_ptr(STI::Device::RunnableTask);

%template(TaskVector) std::vector< std::shared_ptr< STI::Utils::Task > >;

%include "RunnableTask.h"

%include "sti/utils/Task.h"

%ignore STI::Utils::IntervalTask::IntervalTask(const std::string& id, double wait_seconds, const std::function<void(void)>& runFunc);
%ignore STI::Utils::IntervalTask::IntervalTask(const std::string& id, const std::string& wait_time, const std::function<void(void)>& runFunc);
%include "sti/utils/IntervalTask.h"
%extend STI::Utils::IntervalTask 
{
    IntervalTask(const std::string& id, double wait_seconds, const std::shared_ptr< STI::Device::RunnableTask >& runnable)
    {
        STI::Utils::IntervalTask* task = new STI::Utils::IntervalTask(id, wait_seconds, [runnable]() { return runnable->run(); });
        return task;
    }

    IntervalTask(const std::string& id, const std::string& wait_time, const std::shared_ptr< STI::Device::RunnableTask >& runnable)
    {
        STI::Utils::IntervalTask* task = new STI::Utils::IntervalTask(id, wait_time, [runnable]() { return runnable->run(); });
        return task;
    }
}

%ignore STI::Utils::AppointmentTask::AppointmentTask(const std::string& id, const std::string& timeOfDay, const std::function<void(void)>& runFunc);
%ignore STI::Utils::AppointmentTask::AppointmentTask(const std::string& id, const std::string& timeOfDay, const AppointmentRepeatType& repeatType, const std::function<void(void)>& runFunc);
%include "sti/utils/AppointmentTask.h"
%extend STI::Utils::AppointmentTask 
{
    AppointmentTask(const std::string& id, const std::string& timeOfDay, const std::shared_ptr< STI::Device::RunnableTask >& runnable)
    {
        STI::Utils::AppointmentTask* task = new STI::Utils::AppointmentTask(id, timeOfDay, [runnable]() { return runnable->run(); });
        return task;
    }

    AppointmentTask(const std::string& id, const std::string& timeOfDay, const STI::Utils::AppointmentTask::AppointmentRepeatType& repeatType, const std::shared_ptr< STI::Device::RunnableTask >& runnable)
    {
        STI::Utils::AppointmentTask* task = new STI::Utils::AppointmentTask(id, timeOfDay, repeatType, [runnable]() { return runnable->run(); });
        return task;
    }
}


%ignore STI::Device::TaskManager;
%ignore STI::Device::JTaskManager::JTaskManager(const std::shared_ptr< STI::Device::TaskManager >& manager);
%include "JTaskManager.h"
