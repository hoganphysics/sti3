#include <sti/device/LogManager.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogRecord.h>
#include <sti/device/Logger.h>

#include "MixedValuePy.h"

#include <sstream>
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/functional.h>
namespace py = pybind11;

using STI::Device::LogManager;
using STI::Device::LogFileFilter;
using STI::Device::LogID;
using STI::Device::LogFile;
using STI::Device::LogRecord;
using STI::Device::DeviceID;
using STI::Device::Logger;


void init_LogManager(py::module& m)
{
    py::class_<LogFileFilter>(m, "LogFileFilter")
        .def(py::init<>())
        .def_readwrite("logName", &LogFileFilter::logName)
        .def_readwrite("startDate", &LogFileFilter::startDate)
        .def_readwrite("endDate", &LogFileFilter::endDate)
        .def_readwrite("startIndex", &LogFileFilter::startIndex)
        .def_readwrite("endIndex", &LogFileFilter::endIndex)
        .def("__repr__",
            [](const LogFileFilter& id) {
                // <LogFileFilter | log | {4/6/23, 4/7/23} | {0, 4}>
                std::stringstream s;
                s << "<LogFileFilter | " 
                  << id.logName << " | " 
                  << "{" << id.startDate << " | " 
                  << ", " << id.endDate << "} | {" 
                  << id.startIndex << ", " << id.endIndex << "}>";
                return s.str();
            })
        ;


    py::class_<Logger, std::shared_ptr<Logger>>(m, "Logger")
        .def("name", &Logger::getName)
        .def("addLogTask", 
            [](const std::shared_ptr<Logger>& self, const std::string& timeInterval, const std::function<std::string(void)>& runFunc) {
                // Need to wrap python function reference in another lambda so we can release the GIL
                // before calling back to python
                auto gil_runFunc = [runFunc]() {
                    std::string result = "";
                    {
                        pybind11::gil_scoped_acquire acquire;
                        result = runFunc();
                    }
                    return result;
                };
                self->addLogTask(timeInterval, gil_runFunc);
            }, py::arg("timeInterval"), py::arg("runFunc"))

        .def("__addReadLogTask", py::overload_cast<short, const std::string&>(&Logger::addReadLogTask), 
                py::arg("channel"), py::arg("timeInterval"))
        // .def("addReadLogTask", py::overload_cast<short, const std::string&, const STI::Utils::MixedValue&>(&Logger::addReadLogTask), 
        //         py::arg("channel"), py::arg("timeInterval"), py::arg("value"))
        
        .def("__addReadLogTask_value", (
            [](const std::shared_ptr<Logger>& self, short channel, const std::string& timeInterval, const py::object& value) {
                STI::Python::MixedValuePy mixedValue(value);
                self->addReadLogTask(channel, timeInterval, mixedValue.getMixedValue());
            }), py::arg("channel"), py::arg("timeInterval"), py::arg("value"))

        // .def("addReadLogTask",py::overload_cast<Logger&, short, const std::string&, const std::function<STI::Utils::MixedValue(void)>&>(
        .def("__addReadLogTask_callable", (
            [](const std::shared_ptr<Logger>& self, short channel, const std::string& timeInterval, const std::function<py::object(void)>& runFunc) {
                // Need to wrap python function reference in another lambda so we can release the GIL
                // before calling back to python
                auto gil_runFunc = [runFunc]() -> STI::Utils::MixedValue {
                    STI::Python::MixedValuePy result;
                    {
                        pybind11::gil_scoped_acquire acquire;
                        auto pyResult = runFunc();
                        result.setValue_py(pyResult);
                    }
                    return result.getMixedValue();
                };
                self->addReadLogTask(channel, timeInterval, gil_runFunc);
            }), py::arg("channel"), py::arg("timeInterval"), py::arg("runFunc"))

        // .def("addWriteLogTask", py::overload_cast<short, const std::string&, const STI::Utils::MixedValue&>(&Logger::addWriteLogTask), 
        //         py::arg("channel"), py::arg("timeInterval"), py::arg("value"))
        .def("__addWriteLogTask_value", (
            [](const std::shared_ptr<Logger>& self, short channel, const std::string& timeInterval, const py::object& value) {
                STI::Python::MixedValuePy mixedValue(value);
                self->addWriteLogTask(channel, timeInterval, mixedValue.getMixedValue());
            }), py::arg("channel"), py::arg("timeInterval"), py::arg("value"))

        .def("__addWriteLogTask_callable", (
            [](const std::shared_ptr<Logger>& self, short channel, const std::string& timeInterval, const std::function<py::object(void)>& runFunc) {
                // Need to wrap python function reference in another lambda so we can release the GIL
                // before calling back to python
                auto gil_runFunc = [runFunc]() -> STI::Utils::MixedValue {
                    STI::Python::MixedValuePy result;
                    {
                        pybind11::gil_scoped_acquire acquire;
                        auto pyResult = runFunc();
                        result.setValue_py(pyResult);
                    }
                    return result.getMixedValue();
                };               
                self->addWriteLogTask(channel, timeInterval, gil_runFunc);
            }), py::arg("channel"), py::arg("timeInterval"), py::arg("runFunc"))

        .def("addAttributeLogTask", &Logger::addAttributeLogTask, py::arg("key"), py::arg("timeInterval"))
        .def("append", 
            [](const std::shared_ptr<Logger>& self, const std::string& message) {
                (*self) << message;
                return self;
            })
        ;

    py::class_<LogManager, std::shared_ptr<LogManager>>(m, "LogManager")
        .def("getLogNames", 
            [](LogManager& self) {
                std::set<std::string> names;
                self.getLogNames(names);
                return names;
            })
        .def("getLogCount", py::overload_cast<const LogFileFilter&>(&LogManager::getLogCount), 
                py::arg("filter"))
        .def("getLogCount", py::overload_cast<const DeviceID&, const LogFileFilter&>(&LogManager::getLogCount), 
                py::arg("deviceID"), py::arg("filter"))
        .def("getLogIDs", 
            [](LogManager& self, const LogFileFilter& filter) {
                std::vector<LogID> ids;
                self.getLogIDs(filter, ids);
                return ids;
            }, py::arg("filter"))
        // .def("getLog", 
        //     [](LogManager& self, const LogID& id) {
        //         LogFile logFile;
        //         self.getLog(id, logFile);
        //         return logFile;
        //     }, py::arg("logID"))
        
        .def("getLogRecord", 
            [](LogManager& self, const std::string& date) {
                LogRecord record;
                self.getLogRecord(date, record);
                return record;
            }, py::arg("date"))

        // .def("__repr__",
        //     [](const LogManager& record) {
        //         std::stringstream s;
        //         return s.str();
        //     })
        ;



}

