

#include <iostream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using std::cout;
using std::endl;

void init_DeviceID(pybind11::module &);
void init_EngineID(py::module& m) ;
void init_DeviceMessage(py::module& m);
void init_DeviceMessageDispatcher(py::module& m);
void init_Channel(py::module& m);
void init_MixedValue(py::module& m);
void init_ChannelManager(py::module& m);
void init_SynchronousEvent(py::module& m);
void init_LocalDevice(py::module& m);
void init_DeviceCollection(py::module& m);
void init_EventEngineScheduler(py::module& m);
void init_Attribute(py::module& m);
void init_RawEvent(py::module& m);
void init_AttributeManager(py::module& m);
void init_PersistenceManager(py::module& m);

int add(int i, int j) {
    return i + j;
}

class Animal {
public:
    virtual ~Animal() { }
    virtual std::string go(int n_times) = 0;
    virtual std::string go2(double n_times) = 0;
};

class PyAnimal : public Animal {
public:
    /* Inherit the constructors */
    using Animal::Animal;

    /* Trampoline (need one for each virtual function) */
    std::string go(int n_times) override {
        PYBIND11_OVERLOAD_PURE(
            std::string, /* Return type */
            Animal,      /* Parent class */
            go,          /* Name of function in C++ (must match Python name) */
            n_times      /* Argument(s) */
        );
    }

    std::string go2(double n_times) override {
        PYBIND11_OVERLOAD_PURE(
            std::string, /* Return type */
            Animal,      /* Parent class */
            go2,          /* Name of function in C++ (must match Python name) */
            n_times      /* Argument(s) */
        );
    }
};

class Dog : public Animal {
public:
    std::string go(int n_times) override {
        std::string result;
        for(int i=0; i<n_times; ++i)
            result += "woof! ";
        return result;
    }

    std::string go2(double n_times) override {
        std::string result;
        for(int i=0; i<5; ++i)
            result += "hi! ";
        return result;
    }

};

std::string call_go(Animal *animal) {
    return animal->go(3);
}


// PYBIND11_MODULE(example, m) {
//     m.doc() = "pybind11 example plugin"; // optional module docstring

//     m.def("add", &add, "A function which adds two numbers");
// }


PYBIND11_MODULE(stidevicepy, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("add", &add, "A function which adds two numbers");

    py::class_<Animal, PyAnimal /* <--- trampoline*/>(m, "Animal")
        .def(py::init<>())
        .def("go", &Animal::go)
        .def("go2", &Animal::go2);

    py::class_<Dog, Animal>(m, "Dog")
        .def(py::init<>());

    m.def("call_go", &call_go);

    // py::module_::import("stidevicepybase");

    init_DeviceID(m);
    init_EngineID(m);
    init_DeviceMessage(m);
    init_DeviceMessageDispatcher(m);
    init_Channel(m);
    init_MixedValue(m);
    init_ChannelManager(m);
    init_DeviceCollection(m);
    init_SynchronousEvent(m);
    init_LocalDevice(m);
    init_RawEvent(m);
    init_EventEngineScheduler(m);
    init_Attribute(m);
    init_AttributeManager(m);
    init_PersistenceManager(m);
}

int main(int argc, char *argv[])
{

//    cout << "Test" << endl;

    return 0;
}

