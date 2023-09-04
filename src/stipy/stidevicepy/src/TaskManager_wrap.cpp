#include <sti/device/TaskManager.h>

#include <sti/utils/Task.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

namespace py = pybind11;

using STI::Utils::Task;
using STI::Device::TaskManager;


void init_TaskManager(py::module& m)
{

    py::class_<TaskManager, std::shared_ptr<TaskManager>>(m, "TaskManager")
        .def("getTaskIDs",
            [](const TaskManager& self) {
                std::set<std::string> ids;
                self.getTaskIDs(ids);
                return ids;
            })
        .def("getTaskStatus", &TaskManager::getTaskStatus, py::arg("taskID"))

        .def("getTask",
            [](const TaskManager& self, const std::string& taskID) {
                std::shared_ptr<STI::Utils::Task> task;
                self.getTask(taskID, task);
                return task;
            }, py::arg("taskID"))
        .def("getTasks",
            [](const TaskManager& self) {
                std::vector<std::shared_ptr<STI::Utils::Task>> tasks;
                self.getTasks(tasks);
                return tasks;
            })
        .def("removeTask", &TaskManager::removeTask, py::arg("taskID"))
        .def("clear", &TaskManager::clear)
        .def("activateTask", &TaskManager::activateTask, py::arg("taskID"))
        .def("deactivateTask", &TaskManager::deactivateTask, py::arg("taskID"))
        .def("runTask", &TaskManager::runTask, py::arg("taskID"))
        ;

}

