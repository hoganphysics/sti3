#include <sti/utils/Configuration.h>
#include <sti/utils/ConfigFile.h>

#include <string>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
namespace py = pybind11;

using STI::Utils::Configuration;
using STI::Utils::ConfigFile;


void init_Configuration(py::module& m) 
{

    py::class_<Configuration>(m, "Configuration")
        .def(py::init<>())
        .def(py::init<const std::map<std::string, std::string>& >(), 
                     py::arg("section") )
        .def(py::init<const std::map<std::string, std::map<std::string, std::string>>&>(), 
                     py::arg("config") )
        .def("getSectionNames", &Configuration::getSectionNames)
        .def("getParameterNames", py::overload_cast<>(&Configuration::getParameterNames, py::const_))
        .def("getParameterNames", py::overload_cast<const std::string&>(&Configuration::getParameterNames, py::const_), py::arg("section"))

        .def("getParameters", &Configuration::getParameters, py::arg("section"))
        .def("includes", py::overload_cast<const std::string&>(&Configuration::includes, py::const_), py::arg("name"))
        .def("includes", py::overload_cast<const std::string&, const std::string&>(&Configuration::includes, py::const_), 
                        py::arg("section"), py::arg("name"))

        .def("isList", py::overload_cast<const std::string&>(&Configuration::isList, py::const_), py::arg("name"))
        .def("isList", py::overload_cast<const std::string&, const std::string&>(&Configuration::isList, py::const_), 
                        py::arg("section"), py::arg("name"))
        .def("getList", py::overload_cast<const std::string&>(&Configuration::getList, py::const_), py::arg("name"))
        .def("getList", py::overload_cast<const std::string&, const std::string&>(&Configuration::getList, py::const_), 
                        py::arg("section"), py::arg("name"))

        .def("get", [](Configuration& self, const std::string& name) 
            { 
                std::string value;
                if (!self.getParameter(name, value)) {
                    throw py::key_error();
                }             
                return value;
            }, 
            py::arg("name") )
        .def("get", [](Configuration& self, const std::string& section, const std::string& name) 
            {
                std::string value;
                if (!self.getParameter(section, name, value)) {
                    throw py::key_error();
                }             
                return value;
            }, 
            py::arg("section"), py::arg("name") )
        .def("set", [](Configuration& self, const std::string& name, const std::string& value) 
            { 
                return self.set(name, value);
            }, 
            py::arg("name"), py::arg("value") )
        .def("set", [](Configuration& self, const std::string& section, const std::string& name, const std::string& value) 
            {
                return self.set(section, name, value);
            }, 
            py::arg("section"), py::arg("name"), py::arg("value") )
        .def("addToList", [](Configuration& self, const std::string& name, const std::string& value) 
            { 
                return self.addToList(name, value);
            }, 
            py::arg("name"), py::arg("value") )
        .def("addToList", [](Configuration& self, const std::string& section, const std::string& name, const std::string& value) 
            {
                return self.addToList(section, name, value);
            }, 
            py::arg("section"), py::arg("name"), py::arg("value") )
        .def("append", py::overload_cast<const Configuration&>(&Configuration::append), py::arg("config"))
        .def("append", py::overload_cast<const std::map<std::string, std::map<std::string, std::string>>&>(&Configuration::append), 
                        py::arg("config"))
        .def("__add__",
            [](Configuration& self, const Configuration& other) {
                return self + other;
            })
        .def("__add__",
            [](Configuration& self, const std::map<std::string, std::map<std::string, std::string>>& other) {
                return self + other;
            })
        .def("__repr__",
            [](const Configuration& self) {
                std::stringstream repr;
                repr << "| ";

                auto sectionNames = self.getSectionNames();
                for (auto& section : sectionNames) {
                    repr << section << " | ";
                }
                return repr.str();
            })
        ;


    py::class_<ConfigFile>(m, "ConfigFile")
        .def(py::init<>())
        .def(py::init<const std::string&>(), py::arg("filename") )
        .def("save", &ConfigFile::save)
        .def("load", py::overload_cast<>(&ConfigFile::load))
        .def("load", py::overload_cast<const std::string&>(&ConfigFile::load), py::arg("filename"))
        .def("isParsed", &ConfigFile::isParsed)
        .def("setHeader", &ConfigFile::setHeader)
        ;

}

