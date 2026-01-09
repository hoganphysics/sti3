#include "ProfilePy.h"

#include "MixedValuePy.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>

#include <memory>

//using STI::Device::Profile;
using STI::Python::ProfilePy;
using STI::Device::ProfileType;

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::map<std::string, std::string>);
PYBIND11_MAKE_OPAQUE(std::map<short, STI::Python::MixedValuePy>);

void init_Profile(py::module& m)
{

    py::bind_map<std::map<std::string, std::string>>(m, "MapStringString");
    py::bind_map<std::map<short, STI::Python::MixedValuePy>>(m, "MapShortMixedValue")
        .def("__repr__",
            [](const std::map<short, STI::Python::MixedValuePy>& self) {
                std::stringstream s;
                s << "{";
                bool start = true;
                for (auto& item : self) {
                    if (!start) {
                        s << ", ";
                    }
                    s << item.first;
                    s << ": ";
                    s << item.second.print();
                    start = false;
                }
                s << "}";
                return s.str();
            });

    py::enum_<ProfileType>(m, "ProfileType")
        .value("Attribute", ProfileType::Attribute)
        .value("Channel", ProfileType::Channel)
        .value("All", ProfileType::All)
        ;


    py::class_<ProfilePy, std::shared_ptr<ProfilePy>>(m, "Profile")
        .def(py::init<>())
        .def(py::init<std::string>(), py::arg("name"))
        .def_readwrite("name", &ProfilePy::name)
        .def_readwrite("type", &ProfilePy::type)
        .def_readwrite("readOnly", &ProfilePy::readOnly)
        .def_readwrite("attributeData", &ProfilePy::attributeData)
        .def_readwrite("channelData", &ProfilePy::channelData)

        .def("__repr__",
            [](const ProfilePy& self) {
                std::stringstream s;
                // "<Profile | 'Startup'>"
                s << "<Profile | '" << self.name;
                
                if (self.readOnly) {
                    s << "' (read-only)";
                }

                s << "'>";     
                return s.str();
            })
        ;

}
