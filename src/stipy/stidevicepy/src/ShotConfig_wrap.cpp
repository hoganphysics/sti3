
#include <sti/engine/ShotConfig.h>

#include <pybind11/pybind11.h>

using STI::Engine::ShotConfig;
using STI::Engine::ShotType;

namespace py = pybind11;

void init_ShotConfig(py::module& m)
{

    py::enum_<ShotType>(m, "ShotType")
        .value("SingleShot", ShotType::Single)
        .value("SequenceShot", ShotType::Sequence)
        .value("SingleUndocumented", ShotType::SingleUndocumented)
        .export_values();


    py::class_<STI::Engine::ShotConfig>(m, "ShotConfig")
        .def(py::init<>())
        // .def("type", [](const ShotConfig& self) {
        //         return printShotType(self.shotType);
        //     })
        .def_readonly("shotType", &ShotConfig::shotType)
        .def_readonly("jobSourceID", &ShotConfig::jobSourceID)
        .def_readonly("targetEnginePool", &ShotConfig::targetEnginePool)
        .def_readonly("file", &ShotConfig::file)
        .def_readonly("comment", &ShotConfig::comment)

        .def("__repr__",
            [](const ShotConfig& self) {
                return self.print();
            })
        ;

}
