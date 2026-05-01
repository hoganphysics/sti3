#include <sti/device/VersionInfo.h>
#include <sti/device/VersionManager.h>

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Device::VersionInfo;
using STI::Device::VersionManager;

void init_Version(py::module& m)
{
    py::class_<VersionInfo>(m, "VersionInfo")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>(),
            py::arg("component"), py::arg("version"))
        .def_readwrite("component", &VersionInfo::component)
        .def_readwrite("version", &VersionInfo::version)
        .def_readwrite("major", &VersionInfo::major)
        .def_readwrite("minor", &VersionInfo::minor)
        .def_readwrite("patch", &VersionInfo::patch)
        .def_readwrite("buildNumber", &VersionInfo::buildNumber)
        .def_readwrite("buildString", &VersionInfo::buildString)
        .def_readwrite("gitCommit", &VersionInfo::gitCommit)
        .def_readwrite("gitDirty", &VersionInfo::gitDirty)
        .def_readwrite("metadata", &VersionInfo::metadata)
        .def("empty", &VersionInfo::empty)
        .def("toString", &VersionInfo::toString)
        .def("__repr__", [](const VersionInfo& self) {
            return self.toString();
        })
        ;

    py::class_<VersionManager, std::shared_ptr<VersionManager>>(m, "VersionManager")
        .def("getVersions", [](const VersionManager& self) {
            return self.getVersions();
        })
        .def("getVersion", [](const VersionManager& self, const std::string& component) -> py::object {
            VersionInfo version;
            if (self.getVersion(component, version)) {
                return py::cast(version);
            }
            return py::none();
        }, py::arg("component"))
        .def("getLibraryVersion", &VersionManager::getLibraryVersion)
        .def("summary", &VersionManager::summary)
        .def("addVersionInfo", &VersionManager::addVersionInfo, py::arg("version"))
        ;

    m.def("getSTILibraryVersion", &STI::Device::getSTILibraryVersion);
    m.def("getSTILibraryVersionString", &STI::Device::getSTILibraryVersionString);
    m.def("getSTILibraryVersionSummary", &STI::Device::getSTILibraryVersionSummary);
}
