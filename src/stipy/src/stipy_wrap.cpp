
#include "stipy.h"

#include "STIPyChannel.h"
#include "STIPyServer.h"
#include "STIPyChannel.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

// using STI::Python::connect4;

using STI::Python::STIPyChannel;



void init_stipy(py::module& m) 
{

    // m.def("connect3", &STI::Python::connect2, "Connect to the STI server");
    
    m.def("connect", 
        py::overload_cast<const std::string&, 
                        const STI::Device::DeviceID&, 
                        const std::string&>(&STI::Python::connect), "Connect to the STI server");
    
    // m.def("add", &add, "A function which adds two numbers");

    m.def("disconnect", &STI::Python::disconnect, "Disconnect from the STI server");

    m.def("printNetwork", &STI::Python::printNetwork, "Print the STI network tree");

    m.def("event", &STI::Python::event);
    m.def("meas", py::overload_cast<const STIPyChannel&, double, const pybind11::object&>(&STI::Python::meas));
    m.def("meas", py::overload_cast<const STIPyChannel&, double>(&STI::Python::meas));





    m.def("dev", 
        py::overload_cast<const std::string&, const std::string&, unsigned, const std::string&>(
            &STI::Python::dev), "Create STIPy device ID");
    m.def("dev", 
        py::overload_cast<const std::string&, const std::string&, unsigned>(
            &STI::Python::dev), "Create STIPy device ID");
    m.def("ch", 
        py::overload_cast<const std::shared_ptr<STI::Python::STIPyDevice>&, unsigned>(
            &STI::Python::ch), "Create STIPy channel ID");


}

