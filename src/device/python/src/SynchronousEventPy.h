
#ifndef STI_PYTHON_SYNCHRONOUSEVENTPY_H
#define STI_PYTHON_SYNCHRONOUSEVENTPY_H

#include "SynchronousEvent.h"

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

class SynchronousEventPy : public STI::Engine::SynchronousEventAdapter
{
public:

    using STI::Engine::SynchronousEventAdapter::SynchronousEventAdapter;  //inherit constructors

    /* Trampoline (need one for each virtual function) */
    void loadEvent() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            loadEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void playEvent() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            playEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void collectMeasurementData() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            collectMeasurementData,         /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void stopEvent() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            stopEvent,                      /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void pauseEvent() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            pauseEvent,                     /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }
    
    void unpauseEvent(bool retrigger) override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            unpauseEvent,                   /* Name of function in C++ (must match Python name) */
            retrigger                       /* Argument(s) */
        );
    }


    void waitBeforePlay() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            waitBeforePlay,                 /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }

    void waitBeforeCollectData() override {
        PYBIND11_OVERLOAD(
            void,                           /* Return type */
            STI::Engine::SynchronousEventAdapter,  /* Parent class */
            waitBeforeCollectData,          /* Name of function in C++ (must match Python name) */
                                            /* Argument(s) */
        );
    }


};



} //Python
} //STI

#endif

