#include <sti/utils/MixedValue.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/Image.h>
#include "MixedValuePy.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::BinaryData;
using STI::Utils::Image;
using STI::Python::MixedValuePy;

namespace
{
std::shared_ptr<BinaryData> makeBinaryData(const py::bytes& payload)
{
    std::string bytes = payload;
    auto data = std::make_shared<BinaryData>();
    char* rawData = nullptr;
    if (!bytes.empty()) {
        rawData = new char[bytes.size()];
        std::copy(bytes.begin(), bytes.end(), rawData);
    }
    data->assign(rawData, bytes.size(), true);
    return data;
}

py::object binaryDataBytes(BinaryData& data)
{
    char* rawData = nullptr;
    if (!data.getBytes(rawData)) {
        return py::none();
    }
    return py::bytes(rawData, data.bytes());
}

bool saveBinaryData(BinaryData& data, const std::string& path)
{
    char* rawData = nullptr;
    if (!data.getBytes(rawData)) {
        return false;
    }

    auto outputPath = std::filesystem::path(path);
    if (!outputPath.parent_path().empty()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }

    std::ofstream output(path, std::ios::binary);
    if (!output.is_open()) {
        return false;
    }

    if (data.bytes() > 0) {
        output.write(rawData, static_cast<std::streamsize>(data.bytes()));
    }
    return output.good();
}

bool saveImage(Image& image, const std::string& path)
{
    std::shared_ptr<BinaryData> data;
    if (image.getData(data) && data != 0) {
        return saveBinaryData(*data, path);
    }

    std::shared_ptr<STI::Utils::FileHolder> file;
    if (image.getFile(file) && file != 0 && file->exists()) {
        auto outputPath = std::filesystem::path(path);
        if (!outputPath.parent_path().empty()) {
            std::filesystem::create_directories(outputPath.parent_path());
        }
        std::error_code ec;
        return std::filesystem::copy_file(
                   file->getFilename(), outputPath,
                   std::filesystem::copy_options::overwrite_existing, ec) && !ec;
    }

    return false;
}
} // namespace


void init_MixedValue(py::module& m) 
{
    //MixedValueType { Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Any}

    py::enum_<MixedValueType>(m, "MixedValueType")
        .value("Empty", MixedValueType::Empty)
        .value("Boolean", MixedValueType::Boolean)
        .value("Int", MixedValueType::Int)
        .value("Double", MixedValueType::Double)
        .value("String", MixedValueType::String)
        .value("Vector", MixedValueType::Vector)
        .value("VectorInt", MixedValueType::VectorInt)
        .value("Binary", MixedValueType::Binary)
        .value("File", MixedValueType::File)
        .value("Image", MixedValueType::Image)
        .value("Number", MixedValueType::Number)
        .value("Any", MixedValueType::Any)
        ;

    py::class_<BinaryData, std::shared_ptr<BinaryData>>(m, "BinaryData")
        .def(py::init<>())
        .def(py::init(&makeBinaryData), py::arg("data"))
        .def("length", &BinaryData::length)
        .def("bytes", &BinaryData::bytes)
        .def("wordsize", &BinaryData::wordsize)
        .def("hasLocalData", &BinaryData::hasLocalData)
        .def("hasStream", &BinaryData::hasStream)
        .def("isMaterialized", &BinaryData::isMaterialized)
        .def("pull", &BinaryData::materialize)
        .def("getBytes", &binaryDataBytes)
        .def("save", &saveBinaryData, py::arg("path"))
        ;

    py::class_<Image, std::shared_ptr<Image>>(m, "Image")
        .def("getFileID", &Image::getFileID)
        .def("getHeight", &Image::getHeight)
        .def("getWidth", &Image::getWidth)
        .def("hasData",
            [](const Image& image) {
                std::shared_ptr<BinaryData> data;
                return image.getData(data);
            })
        .def("hasFile",
            [](const Image& image) {
                std::shared_ptr<STI::Utils::FileHolder> file;
                return image.getFile(file);
            })
        .def("getData",
            [](const Image& image) {
                std::shared_ptr<BinaryData> data;
                image.getData(data);
                return data;
            })
        .def("pullData",
            [](const Image& image) {
                std::shared_ptr<BinaryData> data;
                return image.getData(data) && data != 0 && data->materialize();
            })
        .def("save", &saveImage, py::arg("path"))
        ;

    py::class_<MixedValuePy>(m, "MixedValue")
        .def(py::init<>())
        .def(py::init<const py::object&>(), py::arg("value"))
        .def("getValue", &MixedValuePy::getValue_py)
        .def("getBinary", &MixedValuePy::getBinary_py)
        .def("getImage", &MixedValuePy::getImage_py)
        // .def("getValue",
        //     [](const MixedValuePy& val) {
        //         if (val.getType() == MixedValueType::File) {
        //             return val.getFileID();
        //         }
        //         return val.getValue_py();
        //     })
        .def("setValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::setValue_py), py::arg("MixedValue"))    //, py::keep_alive<1, 2>()
        .def("setValue", py::overload_cast<const py::object&>(&MixedValuePy::setValue_py), py::arg("value"))
        .def("addValue", py::overload_cast<const MixedValuePy&>(&MixedValuePy::addValue_py), py::arg("MixedValue"))
        .def("addValue", py::overload_cast<const py::handle&>(&MixedValuePy::addValue_py), py::arg("value"))
        .def("getType", &MixedValuePy::getType)
        .def("isType", py::overload_cast<const MixedValueType&>(&MixedValuePy::isType, py::const_), py::arg("type"))
        .def("isType", py::overload_cast<const std::vector<MixedValueType>&>(&MixedValuePy::isType, py::const_), py::arg("types"))
        .def("isNumber", &MixedValuePy::isNumber)
        .def("isEmpty", &MixedValuePy::isEmpty)
        .def("clear", &MixedValuePy::clear)
        .def("print", &MixedValuePy::print)
        .def("__len__",
            [](const MixedValuePy& val) {
                return val.getVector().size();
            })
        .def("__getitem__",
            [](const MixedValuePy& val, int index) {
                if (val.getType() != MixedValueType::Vector) {
                    throw py::type_error("Not a Vector");
                }
                if (index < 0) {
                    index += val.getVector().size();
                }
                if (index < 0 || index >= val.getVector().size()) {
                    throw py::index_error("Index out of range");
                }

                // return MixedValuePy::convertValue(val.getVector().at(index));
                return MixedValuePy(val.getVector().at(index));
            })
        // .def("__iter__",
        //     [](const MixedValuePy& val) 
        //     {
        //         if (val.getType() != MixedValueType::Vector) {
        //             throw py::type_error("Attempted to use an iterator on a MixedValue that is not a Vector");
        //         }
        //         // py::list pyList = val.getValue_py();
        //         // return py::make_iterator(val.getVector().begin(), val.getVector().end()); 
        //         // return py::make_iterator(pyList.begin(), pyList.end(), pyList);
        //     }, 
        //     py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def("__repr__",
            [](const MixedValuePy& val) {
                return val.print();
            })
        .def("__eq__",  // operator ==
            [](const MixedValuePy& self, const MixedValuePy& other) {
                return ( static_cast<const MixedValue&>(self) == static_cast<const MixedValue&>(other) );
            })
        ;

}
