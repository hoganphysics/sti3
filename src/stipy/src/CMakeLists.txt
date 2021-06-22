cmake_minimum_required(VERSION 3.10)

if (MSVC)
cmake_policy(SET CMP0091 NEW)
endif (MSVC)

# set the project name
project(stidevicepy VERSION 3.0.0 DESCRIPTION "STIPy python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)
set(STIDEVICEPY_DIR ${STI_BASE_DIR}/src/device/python/src)


if (MSVC)
set(USE_STI_DEBUG_POSTFIX True)
endif (MSVC)

if (USE_STI_DEBUG_POSTFIX)
#set(STI_LIB_POSTFIX "$<$<CONFIG:Debug>:_d>")
set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)


include_directories(".")

link_directories(${STI_BASE_DIR}/lib)

# pybind11 python extension model
# python3 -m pybind11 --cmakedir
# -Dpybind11_ROOT
#add_subdirectory(/home/hogan/.local/lib/python3.6/site-packages/pybind11/share/cmake/pybind11 .)


#find_package(Python COMPONENTS Interpreter Development REQUIRED)

if (MSVC)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif (MSVC)


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

#set_target_properties(stipy PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stipy PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
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
    stidevice${STI_LIB_POSTFIX}
    stinetwork${STI_LIB_POSTFIX}
 PUBLIC stidevicepybase${STI_LIB_POSTFIX}
    # stidevicepybase.cpython-36m-x86_64-linux-gnu
)
