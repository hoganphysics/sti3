
#ifndef STI_PYTHON_SYNCHRONOUSEVENTPY_H
#define STI_PYTHON_SYNCHRONOUSEVENTPY_H

#include "SynchronousEvent.h"
#include "SynchronousEventPyManager.h"

#include "fwd/SynchronousEvent_fwd.h"

#include <pybind11/pybind11.h>

#include <memory>
#include <iostream>

PYBIND11_MAKE_OPAQUE(STI::Engine::SynchronousEventVector);

namespace STI
{
namespace Python
{

// class SynchronousEventPy : public STI::Engine::SynchronousEventAdapter
// {
// public:

//     using STI::Engine::SynchronousEventAdapter::SynchronousEventAdapter;  //inherit constructors

//     /* Trampoline (need one for each virtual function) */
//     void loadEvent() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             loadEvent,                      /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
    
//     void playEvent() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             playEvent,                      /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
    
//     void collectMeasurementData() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             collectMeasurementData,         /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
    
//     void stopEvent() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             stopEvent,                      /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
    
//     void pauseEvent() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             pauseEvent,                     /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
    
//     void unpauseEvent(bool retrigger) override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             unpauseEvent,                   /* Name of function in C++ (must match Python name) */
//             retrigger                       /* Argument(s) */
//         );
//     }


//     void waitBeforePlay() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             waitBeforePlay,                 /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }

//     void waitBeforeCollectData() override {
//         PYBIND11_OVERLOAD(
//             void,                           /* Return type */
//             STI::Engine::SynchronousEventAdapter,  /* Parent class */
//             waitBeforeCollectData,          /* Name of function in C++ (must match Python name) */
//                                             /* Argument(s) */
//         );
//     }
// };

// class SynchronousEventWrapper : public STI::Engine::SynchronousEvent
// {
//     pybind11::object obj;
//     SynchronousEvent* event;

// };

class SynchronousEventPyManager;

class SynchronousEventPyManagerHolder : public STI::Engine::SynchronousEvent
{
public:

    SynchronousEventPyManagerHolder(double time, const std::shared_ptr<SynchronousEventPyManager>& manager) 
    : STI::Engine::SynchronousEvent(time), manager(manager) {}

    virtual ~SynchronousEventPyManagerHolder()
    {
        std::cout << "---------------------SynchronousEventPyManagerHolder" << std::endl;
    }

    std::shared_ptr<SynchronousEventPyManager> manager;

};

// class SynchronousEventPyManager
// {
// public:

//     SynchronousEventPyManager() {}
//     virtual ~SynchronousEventPyManager()
//     {
//         // pySynchronousEventsRefs.clear();
//     }

//     void addPyEvent(const pybind11::object& evt)
//     {
//         // pySynchronousEventsRefs.push_back(evt);
//     }

// private:

//     // std::vector<pybind11::object> pySynchronousEventsRefs;
// };


class SynchronousEventPy : public STI::Engine::SynchronousEvent //, public std::enable_shared_from_this<SynchronousEventPy>
{
public:

    static std::shared_ptr<SynchronousEventPyManager> pyEventManager;   //stores reference to python objects to keep them alive

    // static int temp;

    static void addPyReference(const std::shared_ptr<STI::Engine::SynchronousEvent>& value)
    {
        auto sepy = std::dynamic_pointer_cast<SynchronousEventPy>(value);

        if (sepy) {
            pybind11::object obj = pybind11::cast(value);

            if (pyEventManager != 0) {
                pyEventManager->addPyEvent(obj);
            }
        }
    }

    static void clearPyRefs()
    {
        if (pyEventManager != 0) {
            pyEventManager->clear();
        }
    }

    // pybind11::object obj;   //reference to python object to keep the derived class alive (note, this is a circular dependency! Instance will not be destroyed...)

    // using STI::Engine::SynchronousEvent::SynchronousEvent;  //inherit constructors
    SynchronousEventPy(double time) 
    : STI::Engine::SynchronousEvent(time)
    {
        std::cout << "Create SynchronousEventPy" << std::endl;

        // pybind11::object obj = pybind11::cast(shared_from_this());
        // obj.inc_ref();
        // obj = pybind11::cast(shared_from_this());
        // obj = pybind11::cast(this);

    }

    virtual ~SynchronousEventPy()
    {
        std::cout << "Destroying SynchronousEventPy" << std::endl;

        // pybind11::object obj = pybind11::cast(shared_from_this());
        // obj.dec_ref();
    }

    // void add(const pybind11::object& selfObject)
    // {
    //     obj = selfObject;
    // }

    /* Trampoline (need one for each virtual function) */
    void loadEvent() override 
    {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            loadEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void playEvent() override 
    {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            playEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void collectMeasurementData() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            collectMeasurementData,         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void stopEvent() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            stopEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void pauseEvent() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            pauseEvent,                     /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void unpauseEvent(bool retrigger) override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            unpauseEvent,                   /* Name of function in C++ (must match Python name) */
            retrigger                       /* Argument(s) */
        );
    }


    void waitBeforePlay() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            waitBeforePlay,                 /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void waitBeforeCollectData() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEvent,  /* Parent class */
            waitBeforeCollectData,          /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
};


} //Python
} //STI

#endif

