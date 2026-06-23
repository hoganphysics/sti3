#ifndef STI_PYTHON_TASKPY_H
#define STI_PYTHON_TASKPY_H

#include <sti/utils/Task.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <optional>


namespace STI
{
namespace Python
{

class TaskPy : public STI::Utils::Task
{
public:

    TaskPy(const std::string& id) : Task(id) {}
    virtual ~TaskPy() 
    {
        pybind11::gil_scoped_acquire acquire;
        task_object = pybind11::none();
    }
	virtual bool isReadyToRun() { return true; }		//allows for unscheduled task abort

	virtual double secondsToNextRun() const { return 100; };
	virtual void run() {}
	virtual void skipTask() {}
	virtual bool repeat() { return true; }		//After running, does the task repeat, or is it removed?

    pybind11::object task_object;
};


class TaskPyTrampoline : public TaskPy
{
public:
    /* Inherit the constructors */
    using TaskPy::TaskPy;

    /* Trampoline (need one for each virtual function) */
    bool isReadyToRun() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            bool,                           /* Return type */
            TaskPy,               /* Parent class */
            isReadyToRun,                   /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    double secondsToNextRun() const override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            double,                         /* Return type */
            TaskPy,                         /* Parent class */
            secondsToNextRun,               /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void run() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            TaskPy,                         /* Parent class */
            run,                            /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void skipTask() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            TaskPy,                         /* Parent class */
            skipTask,                       /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    bool repeat() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            bool,                           /* Return type */
            TaskPy,                         /* Parent class */
            repeat,                         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
};


class TaskWrapperPy : public TaskPy
{
public:

    TaskWrapperPy(const std::shared_ptr<STI::Utils::Task>& task) 
    : TaskPy( (task != 0 ? task->getID() : "") ), task(task)
    {
    }

    bool isActive() const 
    {
        return (task != 0 ? task->isActive() : false);
    }

	STI::Utils::TaskStatus getStatus() const
    {
        return (task != 0 ? task->getStatus() : STI::Utils::TaskStatus::Missing);
    }

	double secondsToNextRun() const
    {
        return (task != 0 ? task->secondsToNextRun() : -1);
    };
	
    void run() 
    {
        if (task != 0) { 
            task->runNow();
        }
    }

    STI::Utils::TimeStamp runNow()
    {
        return (task != 0 ? task->runNow() : STI::Utils::Task::runNow());
    }

    bool hasLastRunTime() const override
    {
        return (task != 0 ? task->hasLastRunTime() : STI::Utils::Task::hasLastRunTime());
    }

    std::optional<STI::Utils::TimeStamp> getLastRunTime() const override
    {
        return (task != 0 ? task->getLastRunTime() : STI::Utils::Task::getLastRunTime());
    }

	void skipTask()
    {
        if (task != 0) { 
            task->skipTask();
        }
    }
	
    bool repeat() 
    {
        return (task != 0 ? task->repeat() : false);
    }	

    std::shared_ptr<STI::Utils::Task> task;
};

} //Python
} //STI

#endif
