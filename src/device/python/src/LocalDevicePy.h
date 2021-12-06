
#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include "LocalDevice.h"

//#include "MixedValuePy.h"
#include "DevicePy.h"
#include "DeviceID.h"

#include <memory>

#include <pybind11/pybind11.h>

#include <iostream>

namespace STI
{
namespace Python
{



// class Dog2 : public Animal2 {
// public:
//     std::string go(int n_times) override {
//         std::string result;
//         for(int i=0; i<n_times; ++i)
//             result += "woof! ";
//         return result;
//     }
    
//     std::string getID() override {
//         std::string result;
//         for(int i=0; i<5; ++i)
//             result += "hi! ";
//         return result;
//     }
// };




class DevicePy;
class ChannelManagerPy;
class MixedValuePy;



class A
{
public:

    A(int v) : val(v) 
    {
        std::cout << "Creating A:" << val << std::endl;
    }
    A(const A &p1)
    {
        std::cout << "Copy A:" << val << std::endl;
    }
    virtual ~A() 
    {
        std::cout << "Destroy A:" << val << std::endl;
    }

    virtual void run()
    {
        std::cout << "A:" << val << std::endl;
    }
    int val;
};


class ATrampoline : public A {
public:
    /* Inherit the constructors */
    using A::A;
    virtual ~ATrampoline() {}

    virtual void run() override
    {
        // std::cout << "ATrampoline:run" << std::endl;
{
        pybind11::gil_scoped_acquire gil;  // Acquire the GIL while in this scope.
        // Try to look up the overridden method on the Python side.
        pybind11::function override = pybind11::get_override(this, "run");
        if (override) {  // method is found
            // std::cout << "ATrampoline override found!" << std::endl;
        }
        else {
            // std::cout << "ATrampoline override missing" << std::endl;
        }
}


        PYBIND11_OVERRIDE(
            void,     /* Return type */
            A,       /* Parent class */
            run,          /* Name of function in C++ (must match Python name) */
                    /* Argument(s) */
        );
    }
};

class LocalDevicePy : public DevicePy
{
public:

    LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
    virtual ~LocalDevicePy();

    virtual int test2(int x) { return 5; }

    void runTest();
    virtual void testVector(std::vector<int>& input);

    void runTest2();
    virtual void testVector2(std::vector<std::shared_ptr<A>>& avec);

    virtual bool writeChannel(short channel, const pybind11::object& value);
    virtual pybind11::object readChannel(short channel, const pybind11::object& value);

	bool write(short channel, const pybind11::object& value);
	pybind11::object read(short channel, const pybind11::object& value);
	void stopRW();

    //virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}

    virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}

    void addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
    {
        device->addChannel(channelNumber, type, inputType, outputType, defaultName);
    }
    
    void addEventEngine(const STI::Engine::EngineID& engineID)
    {
        device->addEventEngine(engineID);
    }

    void addPartner(const STI::Device::DeviceID& id)
    {
        device->addPartner(id);
    }

     std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue)
    {
        std::shared_ptr<STI::Device::LocalAttribute> attribute;
        device->addAttribute(key, initialValue, attribute);
        return attribute;
    }

    std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue, const std::vector<std::string>& allowedValues)
    {
        std::shared_ptr<STI::Device::LocalAttribute> attribute;
        device->addAttribute(key, initialValue, allowedValues, attribute);
        return attribute;
    }


    // void addAttribute(const std::string& key, const std::string& initialValue, const pybind11::list& allowedValues)
    // {
    //     device->addAttribute(key, initialValue);
    // }

private:



    class LocalDeviceDelegate : public STI::Device::LocalDevice
    {
    public:

        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const std::string& name, const std::string& address, unsigned short module,
		    const std::string& targetServer)
            : STI::Device::LocalDevice(name, address, module, targetServer), localDevicePy(localDevicePy) {}
        

        bool writeChannel(short channel, const STI::Utils::MixedValue& value)
        {
            //std::cout << "writeChannel" << std::endl;

            STI::Python::MixedValuePy valuePy(value);
            // STI::Python::MixedValuePy valuePy;
            // valuePy.setValue(34);

            bool success = localDevicePy->writeChannel(channel, valuePy.getValue_py());
            return success;
        }

	    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) 
        {
//            std::cout << "readChannel" << std::endl;

            STI::Python::MixedValuePy valuePy(value);
            auto dataPyObj = localDevicePy->readChannel(channel, valuePy.getValue_py());

//            std::cout << "auto dataPyObj" << std::endl;

            //convert result
            STI::Python::MixedValuePy dataPy;
            dataPy.setValue_py(dataPyObj);
//            std::cout << "setValue_py" << std::endl;
            const STI::Utils::MixedValue& ref = dataPy;
//            std::cout << "ref" << std::endl;
            data.setValue(ref);
            //data = dataPy;
//            data.setValue(static_cast<STI::Utils::MixedValue>(dataPy));

//            std::cout << "data.setValue" << std::endl;

            return true;
        }

        void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);
    
    private:

        LocalDevicePy* localDevicePy;
    };

    std::shared_ptr<STI::Device::LocalDevice> device;

};




class LocalDevicePyTrampoline : public LocalDevicePy {
public:
    /* Inherit the constructors */
    using LocalDevicePy::LocalDevicePy;

    void testVector(std::vector<int>& input) override
    {
        //This is needed:
        pybind11::object dummy = pybind11::cast(input, pybind11::return_value_policy::reference);

        PYBIND11_OVERRIDE(
            void,     /* Return type */
            LocalDevicePy,       /* Parent class */
            testVector,          /* Name of function in C++ (must match Python name) */
            input        /* Argument(s) */
        );
    }

    void testVector2(std::vector<std::shared_ptr<A>>& input) override
    {
        pybind11::object dummy = pybind11::cast(input, pybind11::return_value_policy::reference);

        PYBIND11_OVERRIDE(
            void,     /* Return type */
            LocalDevicePy,       /* Parent class */
            testVector2,          /* Name of function in C++ (must match Python name) */
            input        /* Argument(s) */
        );
    }

    int test2(int x) override
    {
        PYBIND11_OVERRIDE(
            int, /* Return type */
            LocalDevicePy,      /* Parent class */
            test2,          /* Name of function in C++ (must match Python name) */
            x                  /* Argument(s) */
        );
    }

    bool writeChannel(short channel, const pybind11::object& value) override
    {
        //pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            bool,                  /* Return type */
            LocalDevicePy,        /* Parent class */
            writeChannel,          /* Name of function in C++ (must match Python name) */
            channel, value         /* Argument(s) */
        );
    }

    pybind11::object readChannel(short channel, const pybind11::object& value) override
    {
        //pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            pybind11::object,     /* Return type */
            LocalDevicePy,       /* Parent class */
            readChannel,          /* Name of function in C++ (must match Python name) */
            channel, value        /* Argument(s) */
        );
    }

    void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) override
    {
//         {
// //        pybind11::gil_scoped_acquire acquire;

//         //auto synchedEventsVec = std::make_shared<STI::Engine::SynchronousEventVector>();
//         auto synchedEventsVec = std::make_shared<std::vector<int>>();
//         synchedEventsVec->push_back(33);

//         pybind11::object dummy = pybind11::cast(synchedEventsVec, pybind11::return_value_policy::reference);   // force re-use in the following call

// //        pybind11::gil_scoped_release release;
//         }
//        pybind11::gil_scoped_acquire acquire;

        pybind11::object dummy = pybind11::cast(synchedEvents, pybind11::return_value_policy::reference);

        PYBIND11_OVERRIDE(
            void,     /* Return type */
            LocalDevicePy,       /* Parent class */
            parseEvents,          /* Name of function in C++ (must match Python name) */
            events, synchedEvents        /* Argument(s) */
        );

        // std::cout << "After override: " << synchedEvents.size() << std::endl;
    }


};





// class LocalDevicePy : public DevicePy, public STI::Device::LocalDevice 
//                                                         //,      //For python wrapper
// //                                          public std::enable_shared_from_this<LocalDevicePy>    //needed to construct DevicePy
// {
// public:

// //    LocalDevicePy();
//     LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
// 		const std::string& targetServer);
//     virtual ~LocalDevicePy();

// //    const STI::Device::DeviceID test() const { return LocalDevice::getID(); }

//     //DevicePy
// //    const STI::Device::DeviceID getIDpy() const override;
// //    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override;
// //    std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler();    
// //    std::shared_ptr<ChannelManagerPy> getChannelManager() override;

//     std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override;
    
//     std::shared_ptr<ChannelManagerPy> getChannelManager() override;

//     STI::Device::DeviceID getIDpy() override;

//     int test2(int x) override
//     {
//         int y = 3*x;
//         return y;
//     }

//     //Hooks to be implemented in python:
// //    virtual bool writeChannelPy(short channel, const STI::Python::MixedValuePy& value) = 0;
// //    virtual pybind11::object readChannelPy(short channel, const STI::Python::MixedValuePy& value) = 0;

// private:

//     //Overrides for STI::Device::LocalDevice
// 	bool writeChannel(short channel, const STI::Utils::MixedValue& value);
// 	bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);


// };


// class LocalDevicePyTrampoline : public LocalDevicePy {
// public:
//     /* Inherit the constructors */
//     using LocalDevicePy::LocalDevicePy;

//     /* Trampoline (need one for each virtual function) */
//     const STI::Device::DeviceID getIDpy() const override { PYBIND11_OVERRIDE(const STI::Device::DeviceID, LocalDevicePy, getIDpy,); }
//     int test2(int x) override { PYBIND11_OVERRIDE(int, LocalDevicePy, test2, x); }



// };




} //Python
} //STI

#endif

