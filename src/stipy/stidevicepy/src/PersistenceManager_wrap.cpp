
#include "PersistenceManagerPy.h"

#include <sti/utils/VirtualFileHolder.h>

#include <sti/engine/Measurement.h>
#include "MixedValuePy.h"
#include <sti/engine/ShotResult.h>
#include <sti/engine/RawEvent.h>

#include <sti/engine/ParseResult.h>
#include <sti/engine/SequenceResult.h>

#include <string>
#include <memory>
#include <chrono>

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>


// PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);



namespace py = pybind11;

using STI::Python::PersistenceManagerPy;
using STI::Python::MixedValuePy;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::RecordStatus;
using STI::Device::ImportedFile;
using STI::Device::ImportCollisionPolicy;
using STI::Device::ImportFileOptions;
using STI::Device::ImportLifetime;
using STI::Device::ImportStorage;


void init_PersistenceManager(py::module& m)
{
    py::enum_<ImportStorage>(m, "ImportStorage")
        .value("DiskTemporary", ImportStorage::DiskTemporary)
        .value("Virtual", ImportStorage::Virtual)
        ;

    py::enum_<ImportCollisionPolicy>(m, "ImportCollisionPolicy")
        .value("Unique", ImportCollisionPolicy::Unique)
        .value("FailIfExists", ImportCollisionPolicy::FailIfExists)
        .value("Replace", ImportCollisionPolicy::Replace)
        ;

    py::enum_<ImportLifetime>(m, "ImportLifetime")
        .value("Handle", ImportLifetime::Handle)
        ;

    py::class_<ImportFileOptions>(m, "ImportFileOptions")
        .def(py::init([](ImportStorage storage,
                         ImportCollisionPolicy collision,
                         ImportLifetime lifetime,
                         unsigned ttlSeconds) {
                ImportFileOptions options;
                options.storage = storage;
                options.collision = collision;
                options.lifetime = lifetime;
                options.ttl = std::chrono::seconds(ttlSeconds);
                return options;
            }),
            py::arg("storage") = ImportStorage::DiskTemporary,
            py::arg("collision") = ImportCollisionPolicy::Unique,
            py::arg("lifetime") = ImportLifetime::Handle,
            py::arg("ttlSeconds") = 600)
        .def_readwrite("storage", &ImportFileOptions::storage)
        .def_readwrite("collision", &ImportFileOptions::collision)
        .def_readwrite("lifetime", &ImportFileOptions::lifetime)
        .def_property("ttlSeconds",
            [](const ImportFileOptions& options) {
                return static_cast<unsigned>(options.ttl.count());
            },
            [](ImportFileOptions& options, unsigned seconds) {
                options.ttl = std::chrono::seconds(seconds);
            })
        ;

    py::class_<ImportedFile, std::shared_ptr<ImportedFile>>(m, "ImportedFile")
        .def("getImportID", &ImportedFile::getImportID)
        .def("getFileID",
            [](const ImportedFile& imported) {
                return imported.getFileID();
            })
        .def_property_readonly("importID", &ImportedFile::getImportID)
        .def_property_readonly("fileID",
            [](const ImportedFile& imported) {
                return imported.getFileID();
            })
        .def("close", &ImportedFile::close)
        .def_property_readonly("closed", &ImportedFile::isClosed)
        .def("__enter__",
            [](ImportedFile& imported) -> ImportedFile& {
                return imported;
            },
            py::return_value_policy::reference_internal)
        .def("__exit__",
            [](ImportedFile& imported, py::object, py::object, py::object) {
                imported.close();
                return false;
            })
        ;

    py::class_<PersistenceManagerPy, std::shared_ptr<PersistenceManagerPy>>(m, "PersistenceManager")
        .def("findShot", &PersistenceManagerPy::findShot, py::arg("shotID"))
        .def("getParseResult", &PersistenceManagerPy::getParseResult, py::arg("parseID"))
        .def("getShotResult", &PersistenceManagerPy::getShotResult, py::arg("shotID"))
        .def("getSequenceResult", &PersistenceManagerPy::getSequenceResult, py::arg("sequenceID"))
        .def("getMeasurements", &PersistenceManagerPy::getMeasurements, py::arg("shotID"))
        .def("makeFileHolder", &PersistenceManagerPy::makeFileHolder, py::arg("path"), py::arg("filename"))
        .def("makeVirtualFileHolder",
            py::overload_cast<const STI::Utils::FileID&>(&PersistenceManagerPy::makeVirtualFileHolder),
            py::arg("fileID"))
        .def("makeVirtualFileHolder",
            py::overload_cast<const std::shared_ptr<STI::Utils::VirtualFileHolder>&>(&PersistenceManagerPy::makeVirtualFileHolder),
            py::arg("backingHolder"))
        .def("makeVirtualFileServer", &PersistenceManagerPy::makeVirtualFileServer)
        .def("getFileServer", &PersistenceManagerPy::getFileServer)
        .def("importFile", &PersistenceManagerPy::importFile,
            py::arg("sourceID"),
            py::arg("sourceServer"),
            py::arg("options") = ImportFileOptions())
        .def("releaseImportedFile", &PersistenceManagerPy::releaseImportedFile, py::arg("importID"))
        .def("getBasePath", &PersistenceManagerPy::getBasePath)
        .def("getTemporaryPath", &PersistenceManagerPy::getTemporaryPath)
        ;

}
