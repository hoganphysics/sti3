
#ifndef STI_PYTHON_SYNCHRONOUSEVENTPY_H
#define STI_PYTHON_SYNCHRONOUSEVENTPY_H

#include <sti/engine/SynchronousEvent.h>
#include "SynchronousEventPyManager.h"

#include <sti/fwd/SynchronousEvent_fwd.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <iostream>

PYBIND11_MAKE_OPAQUE(STI::Engine::SynchronousEventVector);

namespace STI
{
namespace Python
{


class SynchronousEventPyManager;

class SynchronousEventPyManagerHolder : public STI::Engine::SynchronousEventAdapter
{
public:

    SynchronousEventPyManagerHolder(double time, const std::shared_ptr<SynchronousEventPyManager>& manager) 
    : STI::Engine::SynchronousEventAdapter(time), manager(manager) {}

    virtual ~SynchronousEventPyManagerHolder()
    {
    }

    std::shared_ptr<SynchronousEventPyManager> manager;

};


class SynchronousEventPy : public STI::Engine::SynchronousEventAdapter
{
public:

    static std::shared_ptr<SynchronousEventPyManager> pyEventManager;   //stores reference to python objects to keep them alive

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


    using STI::Engine::SynchronousEventAdapter::SynchronousEventAdapter;  //inherit constructors

    virtual ~SynchronousEventPy()
    {
        std::cout << "~SynchronousEventPy()" << std::endl;
    }

    /* Trampoline (need one for each virtual function) */
    void loadEvent() override 
    {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            loadEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void playEvent() override 
    {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            playEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void collectMeasurementData() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            collectMeasurementData,         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void stopEvent() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            stopEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void pauseEvent() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            pauseEvent,                     /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void unpauseEvent(bool retrigger) override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            unpauseEvent,                   /* Name of function in C++ (must match Python name) */
            retrigger                       /* Argument(s) */
        );
    }


    void waitBeforePlay() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            waitBeforePlay,                 /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void waitBeforeCollectData() override {
        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            waitBeforeCollectData,          /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
};


} //Python
} //STI

#endif

