
#ifndef STI_PYTHON_SYNCHRONOUSEVENTPY_H
#define STI_PYTHON_SYNCHRONOUSEVENTPY_H

#include <sti/engine/SynchronousEvent.h>
#include "SynchronousEventPyManager.h"

#include <sti/fwd/SynchronousEvent_fwd.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <mutex>


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

    using STI::Engine::SynchronousEventAdapter::SynchronousEventAdapter;  //inherit constructors

    virtual ~SynchronousEventPy() {}

    static std::mutex pyEventManagerMutex;
    static std::shared_ptr<SynchronousEventPyManager> pyEventManager;   //stores reference to python objects to keep them alive
    
    static void addPyReference(const std::shared_ptr<STI::Engine::SynchronousEvent>& value);
    static void clearPyRefs();


    /* Trampoline (need one for each virtual function) */
    void loadEvent() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            loadEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void playEvent() override 
    {
        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            playEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void collectMeasurementData() override {

        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            collectMeasurementData,         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void stopEvent() override {

        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            stopEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void pauseEvent() override {

        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            pauseEvent,                     /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void unpauseEvent(bool retrigger) override {

        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            unpauseEvent,                   /* Name of function in C++ (must match Python name) */
            retrigger                       /* Argument(s) */
        );
    }


    void waitBeforePlay() override {

        pybind11::gil_scoped_acquire acquire;

        PYBIND11_OVERRIDE(
            void,                           /* Return type */
            SynchronousEventAdapter,  /* Parent class */
            waitBeforePlay,                 /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void waitBeforeCollectData() override {

        pybind11::gil_scoped_acquire acquire;
        
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

