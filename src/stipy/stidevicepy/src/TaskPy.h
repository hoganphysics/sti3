
#ifndef STI_PYTHON_TASKPY_H
#define STI_PYTHON_TASKPY_H

#include <sti/utils/Task.h>


#include <pybind11/pybind11.h>

#include <memory>


// PYBIND11_MAKE_OPAQUE(STI::Engine::SynchronousEventVector);

namespace STI
{
namespace Python
{


class TaskPy : public STI::Utils::Task
{
public:

    TaskPy(const std::string& id) : Task(id) {}
    virtual ~TaskPy() {}

    /* Trampoline (need one for each virtual function) */
    bool isReadyToRun() override 
    {
        PYBIND11_OVERRIDE(
            bool,                           /* Return type */
            STI::Utils::Task,               /* Parent class */
            isReadyToRun,                   /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    double secondsToNextRun() const override 
    {
        PYBIND11_OVERRIDE_PURE(
            double,                         /* Return type */
            STI::Utils::Task,               /* Parent class */
            secondsToNextRun,               /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void run() override 
    {
        PYBIND11_OVERRIDE_PURE(
            void,                           /* Return type */
            STI::Utils::Task,               /* Parent class */
            run,                            /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void skipTask() override 
    {
        PYBIND11_OVERRIDE_PURE(
            void,                           /* Return type */
            STI::Utils::Task,               /* Parent class */
            skipTask,                       /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    bool repeat() override 
    {
        PYBIND11_OVERRIDE_PURE(
            bool,                           /* Return type */
            STI::Utils::Task,               /* Parent class */
            repeat,                         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
};


} //Python
} //STI

#endif

