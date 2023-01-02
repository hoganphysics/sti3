
#include "ParsedDependencyTree.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

using STI::Engine::ParsedDependencyTree;

namespace py = pybind11;


void init_ParsedDependencyTree(py::module& m) 
{
    py::class_<ParsedDependencyTree, std::shared_ptr<ParsedDependencyTree>>(m, "ParsedDependencyTree")
        .def("getNodes",
            [](const ParsedDependencyTree& self) {
                std::vector<STI::Device::DeviceID> nodes;
                self.getNodes(nodes);
                return nodes;
            })
        .def("getDependedentNodes",
            [](const ParsedDependencyTree& self, const STI::Device::DeviceID& node) {
                std::vector<STI::Device::DeviceID> nodes;
                self.getDependedentNodes(node, nodes);
                return nodes;
            })
        // .def("__repr__",
        //     [](const ParsedDependencyTree& self) {
        //         return self.print();
        //     })
        ;

}
