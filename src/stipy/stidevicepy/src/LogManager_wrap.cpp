
#include <sti/device/LogManager.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogRecord.h>


#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::LogManager;
using STI::Device::LogFileFilter;
using STI::Device::LogID;
using STI::Device::LogFile;
using STI::Device::LogRecord;
using STI::Device::DeviceID;


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



    // virtual void getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids) = 0;
    
    // virtual bool getLog(const LogID& id, LogFile& logFile) = 0;
    // virtual bool getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile) = 0;

    // virtual bool getLogs(const LogFileFilter& filter, std::vector<LogFile>& files) = 0;
    // virtual bool getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files) = 0;


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

