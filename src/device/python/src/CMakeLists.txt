cmake_minimum_required(VERSION 3.10)

if (MSVC)
cmake_policy(SET CMP0091 NEW)
endif (MSVC)

# set the project name
project(stidevicepy VERSION 3.0.0 DESCRIPTION "STI device lib python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_DEBUG_POSTFIX "_d")

if (MSVC)
set(USE_STI_DEBUG_POSTFIX True)
endif (MSVC)

if (USE_STI_DEBUG_POSTFIX)
#set(STI_LIB_POSTFIX "$<$<CONFIG:Debug>:_d>")
set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)

# set(STI_LIB_POSTFIX "$<$<CONFIG:Debug>:_d>")


set(STI_BASE_DIR ../../../..)

include_directories(".")

link_directories(${STI_BASE_DIR}/lib)

# pybind11 python extension model
# python3 -m pybind11 --cmakedir
# -Dpybind11_ROOT
#add_subdirectory(/home/hogan/.local/lib/python3.6/site-packages/pybind11/share/cmake/pybind11 .)

find_package(Threads REQUIRED)

if (MSVC)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif (MSVC)


find_package(pybind11 REQUIRED)
message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")

find_package(PythonLibs 3.6.2 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})


pybind11_add_module(stidevicepy 
    AttributeManager_wrap.cpp
    Attribute_wrap.cpp
    AttributeManagerPy.cpp
    ChannelManagerPy.cpp
    ChannelManager_wrap.cpp
    Channel_wrap.cpp
    DeviceCollectionPy.cpp
    DeviceCollection_wrap.cpp
    DeviceID_wrap.cpp
    DeviceHub_wrap.cpp
    DeviceMessageDispatcher_wrap.cpp
    DeviceMessage_wrap.cpp
    DevicePy.cpp
    EventEngineSchedulerPy.cpp
    EventEngineScheduler_wrap.cpp
    EngineID_wrap.cpp
    HubID_wrap.cpp
    LocalDevicePy.cpp
    LocalDevice_wrap.cpp
    LocalShotPy.cpp
    MixedValuePy.cpp
    MixedValue_wrap.cpp
    PersistenceManagerPy.cpp
    PersistenceManager_wrap.cpp
    pywrapper.cpp
    RawEvent_wrap.cpp
    SynchronousEvent_wrap.cpp
    SynchronousEventPy.cpp
    SynchronousEventPyManager.cpp
)


# pybind11_add_module(stidevicepy 
#     ChannelManager_wrap.cpp
#     DeviceCollection_wrap.cpp
#     DeviceID_wrap.cpp
#     DeviceMessageDispatcher_wrap.cpp
#     DeviceMessage_wrap.cpp
#     LocalDevice_wrap.cpp
#     MixedValue_wrap.cpp
#     pywrapper.cpp
#     RawEvent_wrap.cpp
# )

add_dependencies(stidevicepy stidevicepybase)

#set_target_properties(stidevicepy PROPERTIES VERSION ${PROJECT_VERSION})

#set_target_properties(stidevicepy PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
#)

target_include_directories(stidevicepy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
)


#****add_library
#pybind11_add_module  PRIVATE pybind11::headers 
add_library(stidevicepybase SHARED
    AttributeManagerPy.cpp
    ChannelManagerPy.cpp
    DeviceCollectionPy.cpp
    DevicePy.cpp
    EventEngineSchedulerPy.cpp
    LocalDevicePy.cpp
    MixedValuePy.cpp
    PersistenceManagerPy.cpp
    SynchronousEventPy.cpp
    SynchronousEventPyManager.cpp
    
)


#set_target_properties(stidevicepybase PROPERTIES 
#        VERSION ${PROJECT_VERSION}
        # HEADER pybind11::headers
        # PREFIX lib
        # SUFFIX .so
#        NO_SONAME OFF
#)

if (MSVC)
set(STIDEVICEPYBASE_COMPILE_OPTIONS "")
else ()
set(STIDEVICEPYBASE_COMPILE_OPTIONS "-fvisibility=hidden")
endif (MSVC)

#target_compile_options(stidevicepybase PRIVATE -fvisibility=hidden)
target_compile_options(stidevicepybase PRIVATE ${STIDEVICEPYBASE_COMPILE_OPTIONS})

set_target_properties(stidevicepybase PROPERTIES
    VERSION ${PROJECT_VERSION}
    # DEBUG_POSTFIX "_d"
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

if (USE_STI_DEBUG_POSTFIX) 

    set_target_properties(stidevicepybase PROPERTIES
        DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
    )

endif (USE_STI_DEBUG_POSTFIX)

if (MSVC)
set(STIDEVICEPY_COMPILE_OPTIONS "")
else ()
set(STIDEVICEPY_COMPILE_OPTIONS "-fvisibility=hidden")
endif (MSVC)

#target_compile_options(stidevicepy PRIVATE -fvisibility=hidden)
target_compile_options(stidevicepybase PRIVATE ${STIDEVICEPY_COMPILE_OPTIONS})

set_target_properties(stidevicepy PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)



target_include_directories(stidevicepybase PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
)

#****

install(TARGETS stidevicepy stidevicepybase
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#  target_link_libraries(stidevicepybase PRIVATE stidevice)



target_link_libraries(stidevicepybase 
#    pthread 
    Threads::Threads
    pybind11::module 
    pybind11::headers 
    stidevice${STI_LIB_POSTFIX}
    stinetwork${STI_LIB_POSTFIX}
    ${PYTHON_LIBRARIES})

target_link_libraries(stidevicepy 
    PRIVATE stidevice${STI_LIB_POSTFIX} 
    PRIVATE stinetwork${STI_LIB_POSTFIX}
    PRIVATE stidevicepybase${STI_LIB_POSTFIX}
    )


