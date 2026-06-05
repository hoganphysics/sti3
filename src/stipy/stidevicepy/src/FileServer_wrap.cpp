#include <sti/utils/FileServer.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include <sstream>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

using STI::Utils::FileID;
using STI::Utils::FileHolder;
using STI::Utils::LocalFileHolder;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;
using STI::Utils::FileServer;
using STI::Utils::FileTransferType;


void init_FileServer(py::module& m) 
{

    py::enum_<FileTransferType>(m, "FileTransferType")
        .value("Binary", FileTransferType::Binary)
        .value("String", FileTransferType::String)
        ;

    py::class_<FileHolder, std::shared_ptr<FileHolder>>(m, "FileHolder")
        .def("getID", &FileHolder::getID)
        .def("getFilename", &FileHolder::getFilename)
        .def("getFileSize", &FileHolder::getFileSize)
        .def("exists", &FileHolder::exists)
        .def("transferFile", &FileHolder::transferFile, py::arg("destination"))
        .def("maxBufferSize", &FileHolder::maxBufferSize)
        .def("md5Checksum", &FileHolder::md5Checksum)
        .def("write", &FileHolder::write, py::arg("buffer"), py::arg("length"))
        .def("writeText",
            [](FileHolder& self, const std::string& text) {
                return self.write(text.data(), static_cast<unsigned>(text.size()));
            }, py::arg("text"))
        .def("writeBytes",
            [](FileHolder& self, const py::bytes& payload) {
                std::string bytes = payload;
                if (bytes.empty()) {
                    return true;
                }
                return self.write(bytes.data(), static_cast<unsigned>(bytes.size()));
            }, py::arg("data"))
        .def("openFile", &FileHolder::openFile)
        .def("closeFile", &FileHolder::closeFile)
        .def("__eq__", &FileHolder::operator==)
        .def("__ne__", &FileHolder::operator!=)
        .def("__repr__",
            [](const FileHolder& holder) {
                std::stringstream s;
                s << "<FileHolder | " << holder.getID().print() << " | "
                  << holder.getFilename() << " | "
                  << holder.getFileSize() << " bytes>";
                return s.str();
            })
        ;
    py::class_<LocalFileHolder, FileHolder, std::shared_ptr<LocalFileHolder>>(m, "LocalFileHolder")
        .def(py::init<const std::string&, const std::string&, const std::string&>(), 
             py::arg("originID"), py::arg("path"), py::arg("filename"))
        ;

    py::class_<VirtualFileHolder, LocalFileHolder, std::shared_ptr<VirtualFileHolder>>(m, "VirtualFileHolder")
        .def(py::init<const std::string&, const FileID&>(), 
             py::arg("originID"), py::arg("fileID"))
        .def("getBytes",
            [](const VirtualFileHolder& self) {
                const auto bytes = self.getBytes();
                return py::bytes(bytes);
            })
        ;

    py::class_<FileServer, std::shared_ptr<FileServer>>(m, "FileServer")
        .def("findFile", &FileServer::findFile, py::arg("fileID"))
        .def("getFileSize", &FileServer::getFileSize, py::arg("fileID"))
        .def("transferFile", &FileServer::transferFile, 
             py::arg("source"), py::arg("destination"), py::arg("type"))
        .def("transferFilePartial", &FileServer::transferFilePartial, 
             py::arg("source"), py::arg("destination"), py::arg("offset"), py::arg("lines"))
        .def("deleteFile", &FileServer::deleteFile, py::arg("fileID"))
        .def("__repr__",
            [](const FileServer& server) {
                std::stringstream s;
                s << "<FileServer>";
                return s.str();
            })
        ;

    py::class_<VirtualFileServer, FileServer, std::shared_ptr<VirtualFileServer>>(m, "VirtualFileServer")
        .def(py::init<>())
        .def("addFile", &VirtualFileServer::addFile, py::arg("file"))
        ;

}
