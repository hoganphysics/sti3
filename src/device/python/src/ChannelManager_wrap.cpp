
#include "ChannelManagerPy.h"
#include "ChannelManager.h"

#include "Channel.h"
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
        .def("getChannel", &ChannelManagerPy::getChannelPy)
        .def("writeChannel", &ChannelManagerPy::writeChannelPy)
        .def("readChannel", &ChannelManagerPy::readChannelPy)
        ;



}

