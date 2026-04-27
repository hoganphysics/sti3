
#include <sti/device/LogRecord.h>

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::LogRecord;
using STI::Device::DeviceLogRecord;
using STI::Device::LogFileRecord;
using STI::Device::LogNameRecord;
using STI::Device::LogRecordStatus;


void init_LogRecord(py::module& m)
{
    
    py::enum_<LogRecordStatus>(m, "LogRecordStatus")
        .value("Unqueried", LogRecordStatus::Unqueried)
        .value("LogsPresent", LogRecordStatus::LogsPresent)
        .value("NoLogs", LogRecordStatus::NoLogs)
        .value("Error", LogRecordStatus::Error)
        ;

    py::class_<LogFileRecord>(m, "LogFileRecord")
        .def_readonly("id", &LogFileRecord::id)
        .def_readonly("fileID", &LogFileRecord::fileID)
        .def_readonly("bytes", &LogFileRecord::bytes)
        .def_readonly("lineCount", &LogFileRecord::lineCount)
        .def_readonly("firstEntryTime", &LogFileRecord::firstEntryTime)
        .def_readonly("lastEntryTime", &LogFileRecord::lastEntryTime)
        .def("__repr__",
            [](const LogFileRecord& record) {
                std::stringstream s;
                s << "<LogFileRecord | " << record.id.logName
                  << " | " << record.id.date
                  << " | " << record.id.index
                  << " | " << record.bytes << " bytes>";
                return s.str();
            })
        ;

    py::class_<LogNameRecord>(m, "LogNameRecord")
        .def_readonly("logName", &LogNameRecord::logName)
        .def_readonly("files", &LogNameRecord::files)
        .def_readonly("totalBytes", &LogNameRecord::totalBytes)
        .def_readonly("totalLines", &LogNameRecord::totalLines)
        .def_readonly("nextIndex", &LogNameRecord::nextIndex)
        .def_readonly("lastUpdate", &LogNameRecord::lastUpdate)
        .def("__repr__",
            [](const LogNameRecord& record) {
                std::stringstream s;
                s << "<LogNameRecord | " << record.logName
                  << " | files=" << record.files.size()
                  << " | lines=" << record.totalLines << ">";
                return s.str();
            })
        ;

    py::class_<DeviceLogRecord>(m, "DeviceLogRecord")
        .def_readonly("deviceID", &DeviceLogRecord::deviceID)
        .def_readonly("status", &DeviceLogRecord::status)
        .def_readonly("logNames", &DeviceLogRecord::logNames)
        .def_readonly("logs", &DeviceLogRecord::logs)

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
