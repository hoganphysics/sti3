#include "LogBrowser.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::LogBrowser;


void init_LogBrowser(py::module& m)
{
    py::class_<LogBrowser, std::shared_ptr<LogBrowser>>(m, "LogBrowser")
        .def_property_readonly("logID", &LogBrowser::getLogID)
        .def_property_readonly("bytes", &LogBrowser::getBytes)
        .def_property_readonly("lineCount", &LogBrowser::getLineCount)
        .def_property_readonly("firstEntryTime", &LogBrowser::getFirstEntryTime)
        .def_property_readonly("lastEntryTime", &LogBrowser::getLastEntryTime)
        .def_property_readonly("windowStartLine", &LogBrowser::getWindowStartLine)
        .def_property_readonly("windowLineCount", &LogBrowser::getWindowLineCount)
        .def_property_readonly("text", &LogBrowser::getText)
        .def("refreshMetadata",
            [](LogBrowser& self) {
                py::gil_scoped_release release;
                self.refreshMetadata();
            })
        .def("refresh",
            [](LogBrowser& self) {
                py::gil_scoped_release release;
                return self.tail(0);
            })
        .def("tail",
            [](LogBrowser& self, std::size_t lines) {
                py::gil_scoped_release release;
                return self.tail(lines);
            }, py::arg("lines") = 0)
        .def("read",
            [](LogBrowser& self, long offset, std::size_t lines) {
                py::gil_scoped_release release;
                return self.read(offset, lines);
            }, py::arg("offset"), py::arg("lines") = 0)
        .def("pageBackward",
            [](LogBrowser& self, std::size_t lines) {
                py::gil_scoped_release release;
                return self.pageBackward(lines);
            }, py::arg("lines") = 0)
        .def("pageForward",
            [](LogBrowser& self, std::size_t lines) {
                py::gil_scoped_release release;
                return self.pageForward(lines);
            }, py::arg("lines") = 0)
        .def("saveLocal",
            [](LogBrowser& self, const std::string& path) {
                py::gil_scoped_release release;
                return self.saveLocal(path);
            }, py::arg("path"))
        .def("__str__",
            [](const LogBrowser& self) {
                return self.getText();
            })
        .def("__repr__", &LogBrowser::repr)
        ;
}
