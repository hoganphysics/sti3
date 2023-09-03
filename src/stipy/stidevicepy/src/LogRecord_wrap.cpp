
#include <sti/device/LogRecord.h>

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::LogRecord;
using STI::Device::DeviceLogRecord;
using STI::Device::LogRecordStatus;


void init_LogRecord(py::module& m)
{
    //LogRecordStatus { Unqueried, LogsPresent, NoLogs, Error };
    
    py::enum_<LogRecordStatus>(m, "LogRecordStatus")
        .value("Unqueried", LogRecordStatus::Unqueried)
        .value("LogsPresent", LogRecordStatus::LogsPresent)
        .value("NoLogs", LogRecordStatus::NoLogs)
        .value("Error", LogRecordStatus::Error)
        ;

    py::class_<DeviceLogRecord>(m, "DeviceLogRecord")
        .def_readonly("deviceID", &DeviceLogRecord::deviceID)
        .def_readonly("deviceLogRecords", &DeviceLogRecord::status)
        .def_readonly("logNames", &DeviceLogRecord::logNames)

        .def("__repr__",
            [](const DeviceLogRecord& record) {
                std::stringstream s;
                s << "<DeviceLogRecord | " << record.deviceID << " | " 
                    << DeviceLogRecord::logRecordStatusToString(record.status) << ">";
                return s.str();
            })
        ;

    py::class_<LogRecord>(m, "LogRecord")
        .def_readonly("timeStamp", &LogRecord::timeStamp)
        .def_readonly("deviceLogRecords", &LogRecord::deviceLogRecords)

        .def("__repr__",
            [](const LogRecord& record) {
                std::stringstream s;
                s << "<LogRecord | " << record.timeStamp.date_YYYY_MM_DD() << ">";
                return s.str();
            })
        ;


}

