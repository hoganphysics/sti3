#include <sti/device/TaskManager.h>

#include <sti/utils/Task.h>

#include "TaskPy.h"

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
        .def("setStatus", &TaskManager::setStatus, py::arg("taskID"), py::arg("newStatus"))

        .def("getTask",
            [](const TaskManager& self, const std::string& taskID) {
                std::shared_ptr<STI::Utils::Task> task;
                self.getTask(taskID, task);
                std::shared_ptr<STI::Python::TaskPy> taskPy = std::make_shared<STI::Python::TaskWrapperPy>(task);
                return taskPy;
            }, py::arg("taskID"))
        .def("getTasks",
            [](const TaskManager& self) {
                std::vector<std::shared_ptr<STI::Utils::Task>> tasks;
                self.getTasks(tasks);

                std::vector<std::shared_ptr<STI::Python::TaskPy>> tasksPy;

                for (auto& task : tasks) {
                    auto taskPy = std::make_shared<STI::Python::TaskWrapperPy>(task);
                    tasksPy.push_back(taskPy);
                }
                return tasksPy;
            })
        .def("removeTask", &TaskManager::removeTask, py::arg("taskID"))
        .def("clear", &TaskManager::clear)
        .def("activateTask", &TaskManager::activateTask, py::arg("taskID"))
        .def("deactivateTask", &TaskManager::deactivateTask, py::arg("taskID"))
        .def("runTask", &TaskManager::runTask, py::arg("taskID"))
        ;

}

