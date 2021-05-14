cmake_minimum_required(VERSION 3.10)

# set the project name
project(stidevicepy VERSION 3.0.0 DESCRIPTION "STIPy python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)
set(STIDEVICEPY_DIR ${STI_BASE_DIR}/src/device/python/src)

include_directories(".")

link_directories(${STI_BASE_DIR}/lib)

# pybind11 python extension model
# python3 -m pybind11 --cmakedir
# -Dpybind11_ROOT
#add_subdirectory(/home/hogan/.local/lib/python3.6/site-packages/pybind11/share/cmake/pybind11 .)


#find_package(Python COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11 REQUIRED)
message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")

pybind11_add_module(stipy 
    HubID_wrap.cpp
    ParseTicket.cpp
    ParseTicketManager.cpp
    ParseTicket_wrap.cpp
    pywrapper.cpp
    ResultTicket.cpp
    ResultTicket_wrap.cpp
    ResultTicketManager.cpp
    STIPyChannel.cpp
    STIPyChannel_wrap.cpp
    stipy.cpp
    STIPyDevice.cpp
    STIPyDevice_wrap.cpp
    STIPyGlobal.cpp
    STIPyLibDevice.cpp
    STIPySeq.cpp
    STIPyServer.cpp
    STIPyServer_wrap.cpp
    STIPyShot.cpp
    STIPyShot_wrap.cpp
    stipy_wrap.cpp
    Ticket.cpp
    ${STIDEVICEPY_DIR}/MixedValuePy.cpp
    ${STIDEVICEPY_DIR}/DeviceID_wrap.cpp
)

set_target_properties(stipy PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stipy PROPERTIES
    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

target_include_directories(stipy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/device/python/src
    ${STI_BASE_DIR}/src/network/src
)

install(TARGETS stipy
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

target_link_libraries(stipy PRIVATE 
    stidevice 
    stinetwork
 PUBLIC stidevicepybase
    # stidevicepybase.cpython-36m-x86_64-linux-gnu
)
