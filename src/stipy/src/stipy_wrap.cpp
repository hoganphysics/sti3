#include "stipy.h"

#include <sti/device/VersionManager.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/ShotConfig.h>

#include "StackTrace.h"
#include "STIPyServer.h"
#include "STIPyShot.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>


namespace py = pybind11;

// using STI::Python::connect4;

using STI::Engine::RawEventTarget;
// using STI::Python::StackTracePy;
using STI::Engine::StackTrace;


void init_stipy(py::module& m) 
{
    m.attr("__version__") = STI::Device::getSTILibraryVersionString();
    m.def("version", &STI::Device::getSTILibraryVersionSummary, "Return STI version information");
    m.def("printVersion", []() {
        auto summary = STI::Device::getSTILibraryVersionSummary();
        py::print(summary);
        return summary;
    }, "Print STI version information");
    
    m.def("connect", 
        py::overload_cast<const std::string&, 
        const STI::Device::DeviceID&, 
        const std::string&>(&STI::Python::connect), 
        py::arg("localAddress"), py::arg("serverID"), py::arg("nameServerAddress"), 
        "Connect to an STI server");
    
    m.def("connect",
        py::overload_cast<const std::string&,
        const STI::Device::DeviceID&,
        const STI::Network::HubID&,
        const std::string&>(&STI::Python::connect), 
        py::arg("localAddress"), py::arg("serverID"),py::arg("serverHubID"),py::arg("nameServerAddress"), 
        "Connect to an STI server that is located on a specified HubID");

    m.def("connect", 
        py::overload_cast<const std::string&, 
        const STI::Device::DeviceID&, 
        const std::string&, const STI::Utils::Configuration&>(&STI::Python::connect), 
        py::arg("localAddress"), py::arg("serverID"), py::arg("nameServerAddress"), py::arg("config"), 
        "Connect to an STI server");
    
    m.def("connect",
        py::overload_cast<const std::string&,
        const STI::Device::DeviceID&,
        const STI::Network::HubID&,
        const std::string&, const STI::Utils::Configuration&>(&STI::Python::connect), 
        py::arg("localAddress"), py::arg("serverID"),py::arg("serverHubID"),py::arg("nameServerAddress"), py::arg("config"), 
        "Connect to an STI server that is located on a specified HubID");


    // m.def("disconnect", &STI::Python::disconnect, "Disconnect from the STI server");

    m.def("printNetwork", &STI::Python::printNetwork, "Print the STI network tree");

    m.def("makeshot", py::overload_cast<>(&STI::Python::makeShot));
    m.def("makeshot", py::overload_cast<const STI::Engine::ShotType&>(&STI::Python::makeShot),
                    py::arg("shotType"));
    m.def("makeshot", py::overload_cast<const std::function<void(void)>&, const STI::Engine::ShotType&>(&STI::Python::makeShot),
                    py::arg("func"), py::arg("shotType"));
    m.def("makeshot", py::overload_cast<const std::function<void(void)>&, const std::set<STI::Engine::ParsedVar>&, const STI::Engine::ShotType&>(&STI::Python::makeShot),
                    py::arg("func"), py::arg("vars"), py::arg("shotType"));

    m.def("group", &STI::Python::group, 
                    py::arg("name"));

    m.def("var", &STI::Python::var, py::arg("fullVarName"), py::arg("stackTrace"));

    m.def("setvar", &STI::Python::setvar, 
                    py::arg("name"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"));
    m.def("settag", &STI::Python::settag, 
                    py::arg("name"), py::arg("stackTrace"), py::arg("scope"));

    m.def("event", &STI::Python::event, 
                    py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"));
    m.def("meas", py::overload_cast<const RawEventTarget&, double, const pybind11::object&,
                    const StackTrace&, const std::string&>(&STI::Python::meas),
                    py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"));
    m.def("meas", py::overload_cast<const RawEventTarget&, double,
                    const StackTrace&, const std::string&>(&STI::Python::meas),
                    py::arg("channel"), py::arg("time"), py::arg("stackTrace"), py::arg("group"));
    
                    

    m.def("set_trigger", py::overload_cast<const STI::Device::DeviceID&, const StackTrace&>(&STI::Python::set_trigger), py::arg("deviceID"), py::arg("stackTrace"));
    m.def("set_trigger", py::overload_cast<const STI::Engine::RawEventTargetDevice&, const StackTrace&>(&STI::Python::set_trigger), py::arg("device"), py::arg("stackTrace"));

    m.def("dev", 
        py::overload_cast<const std::string&>(
            &STI::Python::dev), py::arg("deviceName"), "Create abstract STIPy device ID");
    m.def("dev", 
        py::overload_cast<const STI::Device::DeviceID&>(
            &STI::Python::dev), py::arg("deviceID"), "Create STIPy device ID");
    m.def("dev", 
        py::overload_cast<const std::string&, const std::string&, unsigned>(
            &STI::Python::dev), py::arg("name"), py::arg("address"), py::arg("module"), "Create STIPy device ID");
    m.def("ch", 
        py::overload_cast<const STI::Engine::RawEventTargetDevice&, unsigned>(
            &STI::Python::ch), py::arg("device"), py::arg("channel"), "Create STIPy channel ID");
    m.def("ch", 
        py::overload_cast<const STI::Engine::RawEventTargetDevice&, const std::string&>(
            &STI::Python::ch), py::arg("device"), py::arg("channelName"), "Create abstract STIPy channel ID");
    m.def("ch", 
        py::overload_cast<const STI::Device::DeviceID&, unsigned>(
            &STI::Python::ch), py::arg("deviceID"), py::arg("channel"), "Create STIPy channel ID");
    m.def("ch", 
        py::overload_cast<const STI::Device::DeviceID&, const std::string&>(
            &STI::Python::ch), py::arg("deviceID"), py::arg("channelName"), "Create abstract STIPy channel ID");
    m.def("ch", 
        py::overload_cast<const std::string&>(
            &STI::Python::ch), py::arg("channelName"), "Create abstract STIPy channel ID");


}
