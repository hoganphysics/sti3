
#include <sti/engine/ShotConfig.h>
#include <sti/engine/EngineJobSourceID.h>

#include <pybind11/pybind11.h>

using STI::Engine::ShotConfig;
using STI::Engine::ShotType;
using STI::Engine::EngineJobSourceID;

namespace py = pybind11;

void init_ShotConfig(py::module& m)
{

    py::enum_<ShotType>(m, "ShotType")
        .value("Single", ShotType::Single)
        .value("Sequence", ShotType::Sequence)
        .value("SingleUndocumented", ShotType::SingleUndocumented)
        .value("SequenceEntry", ShotType::SequenceEntry)
        ;
        //.export_values();


    py::class_<EngineJobSourceID>(m, "EngineJobSourceID")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>(),
            py::arg("user"), py::arg("machine"))
        .def_readwrite("user", &EngineJobSourceID::user)
        .def_readwrite("machine", &EngineJobSourceID::machine)

        .def("__repr__",
            [](const EngineJobSourceID& self) {
                return self.print();
            })
        ;


    py::class_<STI::Engine::ShotConfig>(m, "ShotConfig")
        .def(py::init<>())
        .def(py::init<ShotType, const EngineJobSourceID&, int, const std::string&, const std::string&>(),
            py::arg("shotType"), py::arg("jobSourceID"), py::arg("targetEnginePool"),
            py::arg("file") , py::arg("comment"))
        // .def("type", [](const ShotConfig& self) {
        //         return printShotType(self.shotType);
        //     })
        .def_readwrite("shotType", &ShotConfig::shotType)
        .def_readwrite("jobSourceID", &ShotConfig::jobSourceID)
        .def_readwrite("targetEnginePool", &ShotConfig::targetEnginePool)
        .def_readwrite("file", &ShotConfig::file)
        .def_readwrite("comment", &ShotConfig::comment)

        .def("__repr__",
            [](const ShotConfig& self) {
                return self.print();
            })
        ;

}
