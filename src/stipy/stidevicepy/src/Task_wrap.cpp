#include <sti/utils/Task.h>
#include <sti/utils/MixedValue.h>

#include "TaskPy.h"
#include "MixedValuePy.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>

#include <memory>

using STI::Utils::Task;
using STI::Utils::TaskStatus;
using STI::Utils::MixedValue;
using STI::Python::TaskPy;
using STI::Python::MixedValuePy;

namespace py = pybind11;


void init_Task(py::module& m)
{
    //Active, Inactive, Missing

    py::enum_<TaskStatus>(m, "TaskStatus")
        .value("Active", TaskStatus::Active)
        .value("Inactive", TaskStatus::Inactive)
        .value("Missing", TaskStatus::Missing)
        ;
        //.export_values();


    py::class_<Task, std::shared_ptr<Task>, TaskPy>(m, "Task")
        .def(py::init<std::string>(), py::arg("id"))

        .def("getID", &Task::getID)
        .def("isActive", &Task::isActive)
        .def("getStatus", &Task::getStatus)
        .def("setStatus", &Task::setStatus, py::arg("newStatus"))
        .def("getMetaData", py::overload_cast<>(&Task::getMetaData, py::const_))
        .def("getMetaData", py::overload_cast<const std::string&>(&Task::getMetaData, py::const_), py::arg("key"))
        .def("addMetadata", 
            [](std::shared_ptr<Task>& self, const std::string& key, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                self->addMetaData(key, v);
                return self;
            }, py::arg("key"), py::arg("value") )

        .def("isReadyToRun", &Task::isReadyToRun)
        .def("secondsToNextRun", &Task::secondsToNextRun)
        .def("run", &Task::run)
        .def("skipTask", &Task::skipTask)
        .def("repeat", &Task::repeat)

        .def("__repr__",
            [](const Task& self) {
                std::stringstream s;
                // "<Profile | 'Startup'>"
                s << "<Task | '" << self.getID() << "'>";     
                return s.str();
            })
        .def("__eq__",  // operator ==
            [](const Task& self, const Task& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const Task& self, const Task& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const Task& self, const Task& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;

}
