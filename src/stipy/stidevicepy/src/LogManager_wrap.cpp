#include <sti/device/LogManager.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogRecord.h>
#include <sti/device/Logger.h>

#include "LocalLogManager.h"
#include "MixedValuePy.h"

#include <sstream>
// #include <iostream>

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
using STI::Device::LocalLogManager;


void init_LogManager(py::module& m)
{
    py::enum_<LogFile::LogFileType>(m, "LogFileType")
        .value("FileID", LogFile::LogFileType::FileID)
        .value("FileHolder", LogFile::LogFileType::FileHolder)
        .value("String", LogFile::LogFileType::String)
        ;

    py::class_<LogFile>(m, "LogFile")
        .def(py::init<>())
        .def_readwrite("id", &LogFile::id)
        .def_readwrite("type", &LogFile::type)
        .def_readwrite("fileID", &LogFile::fileID)
        .def_readwrite("fileHolder", &LogFile::fileHolder)
        .def_readwrite("logString", &LogFile::logString)
        .def("__repr__",
            [](const LogFile& logFile) {
                std::stringstream s;
                s << "<LogFile | " << logFile.id.logName
                  << " | " << logFile.id.date
                  << " | " << logFile.id.index << ">";
                return s.str();
            })
        ;

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
        .def("getLogIDs",
            [](LogManager& self, const DeviceID& deviceID, const LogFileFilter& filter) {
                std::vector<LogID> ids;
                self.getLogIDs(deviceID, filter, ids);
                return ids;
            }, py::arg("deviceID"), py::arg("filter"))
        .def("getLog", 
            [](LogManager& self, const LogID& id) -> py::object {
                LogFile logFile;
                if (!self.getLog(id, logFile)) {
                    return py::none();
                }
                return py::cast(logFile);
            }, py::arg("logID"))
        .def("getLog",
            [](LogManager& self, const std::string& name, const std::string& date, unsigned index) -> py::object {
                LogFile logFile;
                if (!self.getLog(name, date, index, logFile)) {
                    return py::none();
                }
                return py::cast(logFile);
            }, py::arg("name"), py::arg("date"), py::arg("index"))
        .def("getLogs",
            [](LogManager& self, const LogFileFilter& filter) {
                std::vector<LogFile> files;
                self.getLogs(filter, files);
                return files;
            }, py::arg("filter"))
        .def("getLogs",
            [](LogManager& self, const DeviceID& deviceID, const LogFileFilter& filter) {
                std::vector<LogFile> files;
                self.getLogs(deviceID, filter, files);
                return files;
            }, py::arg("deviceID"), py::arg("filter"))
        
        .def("getLogRecord", 
            [](LogManager& self, const std::string& date) {
                LogRecord record;
                self.getLogRecord(date, record);
                return record;
            }, py::arg("date"))
        .def("getNetworkLogNames",
            [](LogManager& self) {
                std::set<std::string> names;
                self.getNetworkLogNames(names);
                return names;
            })
        .def("getNetworkLogCount", &LogManager::getNetworkLogCount, py::arg("filter"))
        .def("getNetworkLogIDs",
            [](LogManager& self, const LogFileFilter& filter) {
                std::vector<LogID> ids;
                self.getNetworkLogIDs(filter, ids);
                return ids;
            }, py::arg("filter"))
        .def("getNetworkLogs",
            [](LogManager& self, const LogFileFilter& filter) {
                std::vector<LogFile> files;
                self.getNetworkLogs(filter, files);
                return files;
            }, py::arg("filter"))

        // .def("__repr__",
        //     [](const LogManager& record) {
        //         std::stringstream s;
        //         return s.str();
        //     })
        ;

    py::class_<LocalLogManager, LogManager, std::shared_ptr<LocalLogManager>>(m, "LocalLogManager")
        .def("createLogger", &LocalLogManager::createLogger, py::arg("name"))
        ;


}
