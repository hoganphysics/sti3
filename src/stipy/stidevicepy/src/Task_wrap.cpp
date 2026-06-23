#include <sti/utils/Task.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/IntervalTask.h>
#include <sti/utils/AppointmentTask.h>

#include "TaskPy.h"
#include "MixedValuePy.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>

#include <memory>

using STI::Utils::Task;
using STI::Utils::TaskStatus;
using STI::Utils::MixedValue;
using STI::Python::TaskPy;
using STI::Python::TaskPyTrampoline;
using STI::Python::MixedValuePy;
using STI::Utils::AppointmentTask;
using STI::Utils::IntervalTask;


namespace py = pybind11;


void init_Task(py::module& m)
{
    //Active, Inactive, Missing

    py::enum_<TaskStatus>(m, "TaskStatus")
        .value("Active", TaskStatus::Active)
        .value("Inactive", TaskStatus::Inactive)
        .value("Missing", TaskStatus::Missing)
        ;

    py::class_<Task, std::shared_ptr<Task>>(m, "TaskBase")
        .def("runNow", &Task::runNow)
        .def("hasLastRunTime", &Task::hasLastRunTime)
        .def("getLastRunTime",
            [](const Task& self) -> py::object {
                auto lastRunTime = self.getLastRunTime();
                if (lastRunTime.has_value()) {
                    return py::cast(lastRunTime.value());
                }
                return py::none();
            })
        ;

    py::class_<TaskPy, TaskPyTrampoline, std::shared_ptr<TaskPy>>(m, "Task")
        .def(py::init<std::string>(), py::arg("id"))

        .def("getID", &TaskPy::getID)
        .def("isActive", &TaskPy::isActive)
        .def("getStatus", &TaskPy::getStatus)
        // .def("setStatus", &Task::setStatus, py::arg("newStatus"))
        .def("getMetaData", py::overload_cast<>(&TaskPy::getMetaData, py::const_))
        .def("getMetaData", py::overload_cast<const std::string&>(&TaskPy::getMetaData, py::const_), py::arg("key"))
        .def("addMetadata", 
            [](std::shared_ptr<TaskPy>& self, const std::string& key, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                self->addMetaData(key, v);
                return self;
            }, py::arg("key"), py::arg("value") )
        .def("runNow",
            [](const std::shared_ptr<TaskPy>& self) {
                auto wrapper = std::dynamic_pointer_cast<STI::Python::TaskWrapperPy>(self);
                if (wrapper != 0) {
                    return wrapper->runNow();
                }
                return static_cast<Task*>(self.get())->runNow();
            })
        .def("hasLastRunTime", &TaskPy::hasLastRunTime)
        .def("getLastRunTime",
            [](const TaskPy& self) -> py::object {
                auto lastRunTime = self.getLastRunTime();
                if (lastRunTime.has_value()) {
                    return py::cast(lastRunTime.value());
                }
                return py::none();
            })

        .def("isReadyToRun", &TaskPy::isReadyToRun)
        .def("secondsToNextRun", py::overload_cast<>(&TaskPy::secondsToNextRun, py::const_) )
        .def("run", &TaskPy::run)
        .def("skipTask", &TaskPy::skipTask)
        .def("repeat", &TaskPy::repeat)

        .def("__repr__",
            [](const TaskPy& self) {
                std::stringstream s;
                // "<Profile | 'Startup'>"
                s << "<Task | '" << self.getID() << "'>";     
                return s.str();
            })
        .def("__eq__",  // operator ==
            [](const TaskPy& self, const Task& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const TaskPy& self, const Task& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const TaskPy& self, const Task& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;

    py::class_<IntervalTask, Task, std::shared_ptr<IntervalTask>>(m, "IntervalTask")
        .def(py::init(
            [](const std::string& id, double wait_seconds, const std::function<void(void)>& runFunc) 
                {
                    // Need to wrap python function reference in another lambda so we can acquire the GIL
                    // before calling back to python
                    auto gil_runFunc = [runFunc]() {
                        {
                            pybind11::gil_scoped_acquire acquire;
                            runFunc();
                        }
                    };

                    auto task = std::make_shared<IntervalTask>(id, wait_seconds, gil_runFunc);
                    return task;
                }), 
                py::arg("taskID"), py::arg("wait_seconds"), py::arg("runFunc"))
        .def(py::init(
            [](const std::string& id, const std::string& wait_time, const std::function<void(void)>& runFunc) 
                {
                    // Need to wrap python function reference in another lambda so we can acquire the GIL
                    // before calling back to python
                    auto gil_runFunc = [runFunc]() {
                        {
                            pybind11::gil_scoped_acquire acquire;
                            runFunc();
                        }
                    };

                    auto task = std::make_shared<IntervalTask>(id, wait_time, gil_runFunc);
                    return task;
                }), 
                py::arg("taskID"), py::arg("wait_time"), py::arg("runFunc"))
        ;


    py::enum_<AppointmentTask::AppointmentRepeatType>(m, "AppointmentRepeatType")
        .value("Once", AppointmentTask::AppointmentRepeatType::Once)
        .value("Everyday", AppointmentTask::AppointmentRepeatType::Everyday)
        .value("Weekdays", AppointmentTask::AppointmentRepeatType::Weekdays)
        ;


    py::class_<AppointmentTask, Task, std::shared_ptr<AppointmentTask>>(m, "AppointmentTask")
        .def(py::init(
            [](const std::string& id, const std::string& timeOfDay, const std::function<void(void)>& runFunc) 
                {
                    // Need to wrap python function reference in another lambda so we can acquire the GIL
                    // before calling back to python
                    auto gil_runFunc = [runFunc]() {
                        {
                            pybind11::gil_scoped_acquire acquire;
                            runFunc();
                        }
                    };

                    auto task = std::make_shared<AppointmentTask>(id, timeOfDay, gil_runFunc);
                    return task;
                }), 
                py::arg("taskID"), py::arg("timeOfDay"), py::arg("runFunc"))
        .def(py::init(
            [](const std::string& id, const std::string& timeOfDay, 
                const AppointmentTask::AppointmentRepeatType& repeatType, 
                const std::function<void(void)>& runFunc) 
                {
                    // Need to wrap python function reference in another lambda so we can acquire the GIL
                    // before calling back to python
                    auto gil_runFunc = [runFunc]() {
                        {
                            pybind11::gil_scoped_acquire acquire;
                            runFunc();
                        }
                    };

                    auto task = std::make_shared<AppointmentTask>(id, timeOfDay, repeatType, gil_runFunc);
                    return task;
                }), 
                py::arg("taskID"), py::arg("timeOfDay"), py::arg("repeatType"), py::arg("runFunc"))
        ;

}
