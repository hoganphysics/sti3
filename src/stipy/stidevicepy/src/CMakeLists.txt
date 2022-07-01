cmake_minimum_required(VERSION 3.10...3.20)

#if(MSVC)
#  cmake_policy(SET CMP0091 NEW)
#endif()

# set the project name
project(stidevicepy VERSION 3.0.0 DESCRIPTION "STI device lib python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("Build type: Debug")
    set(USE_STI_DEBUG_POSTFIX True)
  elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message("Build type: RelWithDebInfo")
    set(USE_STI_DEBUG_POSTFIX False)
  else()
    message("Build type: Release")
    set(USE_STI_DEBUG_POSTFIX False)
  endif()
else()
  set(USE_STI_DEBUG_POSTFIX False)
endif()


if (USE_STI_DEBUG_POSTFIX)
  set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)



#set(STI_BASE_DIR ../../../..)

#include_directories(".")


#link_directories(${STI_BASE_DIR}/lib)

# pybind11 python extension model
# python3 -m pybind11 --cmakedir
# -Dpybind11_ROOT
#add_subdirectory(/home/hogan/.local/lib/python3.6/site-packages/pybind11/share/cmake/pybind11 .)

#find_package(Threads REQUIRED)

if (MSVC)
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()


find_package(pybind11 CONFIG REQUIRED)
#message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")

#find_package(PythonLibs 3.6.2 REQUIRED)
#include_directories(${PYTHON_INCLUDE_DIRS})




#****add_library
#pybind11_add_module  PRIVATE pybind11::headers 
#add_library(stipybase SHARED
#    AttributeManagerPy.cpp
#    ChannelManagerPy.cpp
#    DeviceCollectionPy.cpp
#    DevicePy.cpp
#    EventEngineSchedulerPy.cpp
#    LocalDevicePy.cpp
#    MixedValuePy.cpp
#    PersistenceManagerPy.cpp
#    SynchronousEventPy.cpp
#    SynchronousEventPyManager.cpp
#)

# stidevicepy stipy
# stipybase sticommonpy

# Static lib needed by stipybase and stipy
add_library(sticommonpy STATIC
    MixedValuePy.cpp
)

target_include_directories(sticommonpy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${pybind11_INCLUDE_DIRS}
)

set_target_properties(sticommonpy PROPERTIES
#    VERSION ${PROJECT_VERSION}
    # DEBUG_POSTFIX "_d"
    POSITION_INDEPENDENT_CODE TRUE
    #WINDOWS_EXPORT_ALL_SYMBOLS ON
    #CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    #BUILD_SHARED_LIBS TRUE
    #MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)


if (USE_STI_DEBUG_POSTFIX) 
  set_target_properties(sticommonpy PROPERTIES
      DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()



pybind11_add_module(stipybase MODULE
    Configuration_wrap.cpp
    DeviceID_wrap.cpp
	  HubID_wrap.cpp
    MixedValue_wrap.cpp
    pybasewrapper.cpp
    ParseID_wrap.cpp
    RawEvent_wrap.cpp
    ShotConfig_wrap.cpp
)

add_dependencies(stipybase sticommonpy stipybase)

#set_target_properties(stidevicepybase PROPERTIES 
#        VERSION ${PROJECT_VERSION}
        # HEADER pybind11::headers
        # PREFIX lib
        # SUFFIX .so
#        NO_SONAME OFF
#)

if(MSVC)
  set(STIDEVICEPYBASE_COMPILE_OPTIONS "")
else()
  set(STIDEVICEPYBASE_COMPILE_OPTIONS "-fvisibility=hidden")
endif()

#target_compile_options(stidevicepybase PRIVATE -fvisibility=hidden)
target_compile_options(stipybase PRIVATE ${STIDEVICEPYBASE_COMPILE_OPTIONS})



set_target_properties(stipybase PROPERTIES
#    VERSION ${PROJECT_VERSION}
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
  set_target_properties(stipybase PROPERTIES
      DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()



pybind11_add_module(stidevicepy 
    AttributeManager_wrap.cpp
    Attribute_wrap.cpp
    AttributeManagerPy.cpp
    ChannelManagerPy.cpp
    ChannelManager_wrap.cpp
    Channel_wrap.cpp
    DeviceCollectionPy.cpp
    DeviceCollection_wrap.cpp
    DeviceHub_wrap.cpp
    DeviceMessageDispatcher_wrap.cpp
    DeviceMessage_wrap.cpp
    DevicePy.cpp
    EventEngineSchedulerPy.cpp
    EventEngineScheduler_wrap.cpp
    EngineID_wrap.cpp
    LocalDevicePy.cpp
    LocalDevice_wrap.cpp
    LocalShotPy.cpp
    PersistenceManagerPy.cpp
    PersistenceManager_wrap.cpp
    pywrapper.cpp
    SynchronousEvent_wrap.cpp
    SynchronousEventPy.cpp
    SynchronousEventPyManager.cpp
)	

add_dependencies(stidevicepy stipybase)
add_dependencies(stidevicepy sticommonpy)

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

#add_dependencies(stidevicepy stipybase)

#add_dependencies(stidevicepy stidevice)
#add_dependencies(stidevicepy stinetwork)

#set_target_properties(stidevicepy PROPERTIES VERSION ${PROJECT_VERSION})

#set_target_properties(stidevicepy PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
#)

target_include_directories(stidevicepy PRIVATE 
    #${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/src/stipy/stidevicepy/src
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
)


if(MSVC)
  set(STIDEVICEPY_COMPILE_OPTIONS "")
else()
  set(STIDEVICEPY_COMPILE_OPTIONS "-fvisibility=hidden")
endif()

target_compile_options(stidevicepy PRIVATE ${STIDEVICEPY_COMPILE_OPTIONS})

set_target_properties(stidevicepy PROPERTIES
#    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)



target_include_directories(stipybase PRIVATE 
    #${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/src/stipy/stidevicepy/src
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
)




if(MSVC)
  set(STIDEVICEPY_INSTALL_DIR "bin")
else()
  set(STIDEVICEPY_INSTALL_DIR "../../lib")
endif()



#  target_link_libraries(stidevicepybase PRIVATE stidevice)

if(DEFINED SKBUILD)
  install(TARGETS stipybase stidevicepy
    LIBRARY DESTINATION src/stipy/bin
  )
  #Hack? In windows, this install keeps sticommonpy.lib out of the python wheel
  install(TARGETS sticommonpy
    ARCHIVE DESTINATION src/stipy/bin
  )
else()
  install(TARGETS stipybase stidevicepy
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ${STIDEVICEPY_INSTALL_DIR}
    #LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(TARGETS sticommonpy
    #DESTINATION ${STI_BASE_DIR}/lib
    ARCHIVE DESTINATION ${STIDEVICEPY_INSTALL_DIR}
    #LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()


#target_link_libraries(stipybase 
#    Threads::Threads
#    pybind11::module 
#    pybind11::headers 
#    stidevice${STI_LIB_POSTFIX}
#    stinetwork${STI_LIB_POSTFIX}
#    ${PYTHON_LIBRARIES})

target_link_libraries(sticommonpy 
    PRIVATE stidevice${STI_LIB_POSTFIX} 
    )

target_link_libraries(stipybase 
    #PRIVATE stipybase${STI_LIB_POSTFIX}
    PRIVATE stidevice${STI_LIB_POSTFIX}
    PRIVATE stinetwork${STI_LIB_POSTFIX}
    PRIVATE sticommonpy${STI_LIB_POSTFIX}
    )

target_link_libraries(stidevicepy 
    PRIVATE stidevice${STI_LIB_POSTFIX} 
    PRIVATE stinetwork${STI_LIB_POSTFIX} 
    PRIVATE sticommonpy${STI_LIB_POSTFIX}
#    PRIVATE stibasepy${STI_LIB_POSTFIX}
    )



    
