

#include <sti/device/LocalAttribute.h>
#include <sti/device/Attribute.h>
#include "MixedValuePy.h"
#include <sti/utils/utils.h>

#include <string>
#include <memory>
#include <functional>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

namespace py = pybind11;

using STI::Device::Attribute;
using STI::Device::LocalAttribute;
using STI::Utils::MixedValue;
using STI::Python::MixedValuePy;


void init_Attribute(py::module& m) 
{


    py::class_<Attribute, std::shared_ptr<Attribute>>(m, "Attribute")
        .def("getKey", &Attribute::getKey)
        .def("getValue", &Attribute::getValue)
        .def("getAllowedValues", &Attribute::getAllowedValues)
        .def("getGroup", &Attribute::getGroup)
        .def("refreshValue", &Attribute::refreshValue)
        .def("setValue", &Attribute::setValue, py::arg("value"))
        // .def("setValue", py::overload_cast<const std::string&>(&Attribute::setValue))
        // .def("setValue", [](Attribute& self, const std::string& value) {
        //         // return self.setValue(value);
        //         return true;
        //     })
        .def("getMetaData", [](Attribute& self) {
                MixedValuePy value(self.getMetaData());
                return value.getValue_py();
            })
        .def("getMetaData", [](Attribute& self, const std::string& key) {
                MixedValuePy value(self.getMetaData(key));
                return value.getValue_py();
            }, py::arg("key"))
        .def("__repr__",
            [](const Attribute& att) {
                return "<key=" + att.getKey()
                    + ", value=" + att.getValue()
                    + ", group=" + att.getGroup()
                    + ">";
            })
        ;

    py::class_<LocalAttribute, Attribute, std::shared_ptr<LocalAttribute>>(m, "LocalAttribute")
        .def(py::init<const std::string&, const std::string&>(), 
                     py::arg("key"), py::arg("value") )
        .def(py::init(
            [](const std::string& key, const std::string& initalValue, 
              const std::vector<std::string>& allowedValues) 
                {
                    return new STI::Device::LocalAttribute(key, initalValue, allowedValues);
                } ), py::arg("key"), py::arg("value"), py::arg("allowedValues"))
        .def("setRefresher", [](std::shared_ptr<STI::Device::LocalAttribute>& self, const std::function<std::string(void)>& refesher) {
                auto gil_refresher = [refesher]() {
                    pybind11::gil_scoped_release release;
                    return refesher();
                };
                self->setRefresher(gil_refresher);
                //self->setRefresher(refesher);
                return self;
            }, py::arg("refresherFunction"))
        .def("setSetter", [](std::shared_ptr<STI::Device::LocalAttribute>& self, const std::function<bool(const std::string&)>& setter) {
                
                auto gil_setter = [setter](const std::string& value) {
                    pybind11::gil_scoped_release release;
                    return setter(value);
                };
                self->setSetter(gil_setter);
                //self->setSetter(setter);
                return self;
            }, py::arg("setterFunction"))

        // .def("setValue", [&](LocalAttribute& self, const std::string& value) {
        //         return self.setValue<std::string>(value);
        //         // return true;
        //     })
        // .def("setValue", py::overload_cast<const std::string&>(&LocalAttribute::setValue))
        // .def("setRefresher", 
        //     [](LocalAttribute& self, const std::string& key, const MixedValuePy& value) {

        //         std::function<const std::string&(void)> refreshValueCallback;
                
        //         self.setRefresher(refreshValueCallback);
        //         const MixedValue& v = static_cast<const MixedValue&>(value);
        //         //LocalChannel& ch = self.addMetaData(key, v);
        //         //return ch;        //error: use of deleted function ‘STI::Device::LocalChannel::LocalChannel(const STI::Device::LocalChannel&’
        //         self.addMetaData(key, v);
        //         return;
        //     } ) //, py::return_value_policy::reference)
        .def("addMetaData", 
            [](std::shared_ptr<STI::Device::LocalAttribute>& self, const std::string& key, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                //LocalChannel& ch = self.addMetaData(key, v);
                //return ch;        //error: use of deleted function ‘STI::Device::LocalChannel::LocalChannel(const STI::Device::LocalChannel&’
                self->addMetaData(key, v);
                return self;
            }, py::arg("key"), py::arg("value") ) //, py::return_value_policy::reference)
        ;


}

