
#include "ChannelManagerPy.h"
#include <sti/device/ChannelManager.h>

#include <sti/device/Channel.h>
#include "MixedValuePy.h"

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::Channel;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;


void init_ChannelManager(py::module& m) 
{

    py::class_<ChannelManagerPy, std::shared_ptr<ChannelManagerPy>>(m, "ChannelManager")
        .def("getChannels", &ChannelManagerPy::getChannelsPy)
        .def("getChannel", &ChannelManagerPy::getChannelPy, py::arg("channelNumber"))
        .def("writeChannel", &ChannelManagerPy::writeChannelPy, py::arg("channelNumber"), py::arg("value"))
        .def("readChannel", &ChannelManagerPy::readChannelPy, py::arg("channelNumber"), py::arg("value"))
        ;

}

