#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/RawEventGroup.h>

#include <pybind11/pybind11.h>

using STI::Engine::Shot;

namespace py = pybind11;

void init_Shot(py::module& m)
{
    py::class_<Shot, std::shared_ptr<Shot>>(m, "Shot")
        // .def(py::init<>())
        .def("getShotConfig", &Shot::getShotConfig)
        .def("getRootEventGroup",
            [](Shot& self) {
                std::shared_ptr<STI::Engine::RawEventGroup> rootGroup;
                self.getRootEventGroup(rootGroup);
                return rootGroup;
            })
        .def("__repr__",
            [](const Shot& self) {
                return self.getShotConfig().print();
            })
        ;

}
