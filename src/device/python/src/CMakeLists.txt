cmake_minimum_required(VERSION 3.10)

# set the project name
project(stidevicepy VERSION 3.0.0 DESCRIPTION "STI device lib python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../../..)

include_directories(".")

link_directories(${STI_BASE_DIR}/lib)

# pybind11 python extension model
# python3 -m pybind11 --cmakedir
# -Dpybind11_ROOT
#add_subdirectory(/home/hogan/.local/lib/python3.6/site-packages/pybind11/share/cmake/pybind11 .)


#find_package(Python COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11 REQUIRED)
message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")

pybind11_add_module(stidevicepy 
    ChannelManagerPy.cpp
    ChannelManager_wrap.cpp
    Channel_wrap.cpp
    DeviceCollectionPy.cpp
    DeviceCollection_wrap.cpp
    DeviceID_wrap.cpp
    DeviceMessageDispatcher_wrap.cpp
    DeviceMessage_wrap.cpp
    DevicePy.cpp
    EventEngineSchedulerPy.cpp
    LocalDevicePy.cpp
    LocalDevice_wrap.cpp
    MixedValuePy.cpp
    MixedValue_wrap.cpp
    pywrapper.cpp
    RawEvent_wrap.cpp
)

set_target_properties(stidevicepy PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stidevicepy PROPERTIES
    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

target_include_directories(stidevicepy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
)

install(TARGETS stidevicepy
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

target_link_libraries(stidevicepy PRIVATE stidevice)
