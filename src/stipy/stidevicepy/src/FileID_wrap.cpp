#include <sti/utils/FileID.h>
#include <sti/utils/TimeStamp.h>

#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

using STI::Utils::FileID;


void init_FileID(py::module& m) 
{

    py::class_<STI::Utils::FileID>(m, "FileID")
        .def(py::init<>())
        .def_readonly("filename", &FileID::filename)
        .def_readonly("path", &FileID::path)
        .def_readonly("origin", &FileID::origin)
        .def_readonly("persistenceLocation", &FileID::persistenceLocation)
        .def_readonly("creationTime", &FileID::creationTime)
        ;

}

