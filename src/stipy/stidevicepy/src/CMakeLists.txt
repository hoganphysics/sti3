cmake_minimum_required(VERSION 3.10...3.20)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
endif()

# set the project name
project(stidevicepy VERSION ${STI3_VERSION} DESCRIPTION "STI device lib python wrapper" LANGUAGES CXX)

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


if (MSVC)
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

# Default from CMake’s Python package (works for local builds)
set(_PY_DEFAULT "${Python3_SITEARCH}")
if(NOT _PY_DEFAULT)
  set(_PY_DEFAULT "${Python3_SITELIB}")
endif()

# Allow build.sh to override with -DCMAKE_INSTALL_PYTHONDIR="$SP_DIR"
set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_PYTHONDIR}"
    CACHE PATH "Python site-packages install dir")

if(NOT CMAKE_INSTALL_PYTHONDIR)
  set(CMAKE_INSTALL_PYTHONDIR "${_PY_DEFAULT}")
endif()

# Final sanity check so we never get '/stipy/...'
if(NOT CMAKE_INSTALL_PYTHONDIR OR CMAKE_INSTALL_PYTHONDIR STREQUAL "/")
  message(FATAL_ERROR "CMAKE_INSTALL_PYTHONDIR is not set. Pass -DCMAKE_INSTALL_PYTHONDIR=<site-packages> (use \$SP_DIR in conda-build).")
endif()

message(STATUS "CMAKE_INSTALL_PYTHONDIR=${CMAKE_INSTALL_PYTHONDIR}")


find_package(pybind11 CONFIG REQUIRED)

find_package(Boost 1.64 REQUIRED)
include_directories("." ${Boost_INCLUDE_DIR})

# Static lib needed by stipybase and stipy
add_library(sticommonpy STATIC
    AttributeManagerPy.cpp
    ChannelManagerPy.cpp
    DeviceCollectionPy.cpp
    DevicePy.cpp
    LogBrowser.cpp
    MixedValuePy.cpp
    MonitorManagerPy.cpp
    NodeWalkerPy.cpp
    PartnerDevicePy.cpp
    PersistenceManagerPy.cpp
    ProfilePy.cpp
)

target_include_directories(sticommonpy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
    ${STI_BASE_DIR}/src/network/src/generated
    ${pybind11_INCLUDE_DIRS}
)

set_target_properties(sticommonpy PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)


if (USE_STI_DEBUG_POSTFIX) 
  set_target_properties(sticommonpy PROPERTIES
      DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )

endif()

add_dependencies(sticommonpy stidevice)

pybind11_add_module(stipybase MODULE
    AttributeManager_wrap.cpp
    Attribute_wrap.cpp
    Configuration_wrap.cpp
    DeviceID_wrap.cpp
    EngineParsingMessage_wrap.cpp
    EnginePlayingMessage_wrap.cpp
    FileID_wrap.cpp
    FileServer_wrap.cpp
    HubID_wrap.cpp
    LogID_wrap.cpp
    LogRecord_wrap.cpp
    Measurement_wrap.cpp
    MixedValue_wrap.cpp
    pybasewrapper.cpp
    ParseID_wrap.cpp
    ParsedDependencyTree_wrap.cpp
    ParseResult_wrap.cpp
    Profile_wrap.cpp
    RawEvent_wrap.cpp
    RawEventGroup_wrap.cpp
    Sequence_wrap.cpp
    SequenceResult_wrap.cpp
    ShotID_wrap.cpp
    Shot_wrap.cpp
    ShotConfig_wrap.cpp
    ShotResult_wrap.cpp
    StackTrace_wrap.cpp
    Task_wrap.cpp
)



add_dependencies(stipybase sticommonpy stidevice stinetwork)


if(MSVC)
  set(STIDEVICEPYBASE_COMPILE_OPTIONS "")
else()
  set(STIDEVICEPYBASE_COMPILE_OPTIONS "-fvisibility=hidden")
endif()

target_compile_options(stipybase PRIVATE ${STIDEVICEPYBASE_COMPILE_OPTIONS})


target_compile_definitions(stipybase PRIVATE
  PYBIND11_DETAILED_ERROR_MESSAGES=1
  PYBIND11_ASSERT_GIL_HELD_INCREF_DECREF=1  # optional, catches GIL mistakes
)

set_target_properties(stipybase PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    # MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

if (USE_STI_DEBUG_POSTFIX) 
  set_target_properties(stipybase PROPERTIES
      DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()


pybind11_add_module(stidevicepy 
    ChannelManager_wrap.cpp
    Channel_wrap.cpp
    Device_wrap.cpp
    DeviceCollection_wrap.cpp
    DeviceHub_wrap.cpp
    DeviceMessageDispatcher_wrap.cpp
    DeviceMessageReceiver_wrap.cpp
    DeviceMessage_wrap.cpp
    EventEngineScheduler_wrap.cpp
    EngineID_wrap.cpp
    # Exception_wrap.cpp
    LocalDevicePy.cpp
    LocalDevice_wrap.cpp
    LogBrowser_wrap.cpp
    LogManager_wrap.cpp
    PartnerDevice_wrap.cpp
    PersistenceManager_wrap.cpp
    ProfileManager_wrap.cpp
    Monitor_wrap.cpp
    MonitorManager_wrap.cpp
    pywrapper.cpp
    SynchronousEvent_wrap.cpp
    SynchronousEventPy.cpp
    SynchronousEventPyManager.cpp
    TaskManager_wrap.cpp
    Version_wrap.cpp
)	

add_dependencies(stidevicepy stipybase)
add_dependencies(stidevicepy sticommonpy)

target_include_directories(stidevicepy PRIVATE 
    ${STI_BASE_DIR}/src/stipy/stidevicepy/src
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
    ${STI_BASE_DIR}/src/network/src/generated
)


if(MSVC)
  set(STIDEVICEPY_COMPILE_OPTIONS "")
else()
  set(STIDEVICEPY_COMPILE_OPTIONS "-fvisibility=hidden")
endif()

target_compile_options(stidevicepy PRIVATE ${STIDEVICEPY_COMPILE_OPTIONS})

target_compile_definitions(stidevicepy PRIVATE
  PYBIND11_DETAILED_ERROR_MESSAGES=1
  PYBIND11_ASSERT_GIL_HELD_INCREF_DECREF=1  # optional, catches GIL mistakes
)

set_target_properties(stidevicepy PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    # MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)


target_include_directories(stipybase PRIVATE 
    ${STI_BASE_DIR}/src/stipy/stidevicepy/src
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
    ${STI_BASE_DIR}/src/network/src/generated
)

#https://github.com/pybind/cmake_example/issues/11
if(UNIX AND NOT APPLE)
    # set_target_properties(stipybase PROPERTIES INSTALL_RPATH "$ORIGIN/relative/path/to/supporting/library_dir")
    set_target_properties(stipybase PROPERTIES INSTALL_RPATH "$ORIGIN")
    set_target_properties(stidevicepy PROPERTIES INSTALL_RPATH "$ORIGIN")
    message("Setting RPATH in UNIX")
elseif(APPLE)
    set_target_properties(stipybase PROPERTIES INSTALL_RPATH "@loader_path")
    set_target_properties(stidevicepy PROPERTIES INSTALL_RPATH "@loader_path")
endif()

if(MSVC)
    target_link_directories(sticommonpy PUBLIC 
        "${CMAKE_CURRENT_BINARY_DIR}/../../../device/src"
    )
endif()

if(MSVC)
    target_link_directories(stipybase PUBLIC 
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/../../../device/src"
        "${CMAKE_CURRENT_BINARY_DIR}/../../../network/src"
    )
endif()

if(MSVC)
    target_link_directories(stidevicepy PUBLIC 
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/../../../device/src"
        "${CMAKE_CURRENT_BINARY_DIR}/../../../network/src"
    )
endif()


if(MSVC)
  set(STIDEVICEPY_INSTALL_DIR "bin")
else()
  set(STIDEVICEPY_INSTALL_DIR "../../lib")
endif()


target_link_libraries(sticommonpy 
    PRIVATE stidevice${STI_LIB_POSTFIX} 
    )

target_link_libraries(stipybase 
    PRIVATE stidevice${STI_LIB_POSTFIX}
    PRIVATE stinetwork${STI_LIB_POSTFIX}
    PRIVATE sticommonpy${STI_LIB_POSTFIX}
    )

target_link_libraries(stidevicepy 
    PRIVATE stidevice${STI_LIB_POSTFIX} 
    PRIVATE stinetwork${STI_LIB_POSTFIX} 
    PRIVATE sticommonpy${STI_LIB_POSTFIX}
    )

if(DEFINED SKBUILD)
  install(TARGETS stipybase stidevicepy
    LIBRARY DESTINATION src/stipy/bin
  )
  #Hack? In windows, this install keeps sticommonpy.lib out of the python wheel
  install(TARGETS sticommonpy
    ARCHIVE DESTINATION src/stipy/bin
  )
else()
  # install(TARGETS stipybase stidevicepy
  #   #DESTINATION ${STI_BASE_DIR}/lib
  #   LIBRARY DESTINATION ${STIDEVICEPY_INSTALL_DIR}
  #   #LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
  #   #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # )
  # install(TARGETS sticommonpy
  #   #DESTINATION ${STI_BASE_DIR}/lib
  #   ARCHIVE DESTINATION ${STIDEVICEPY_INSTALL_DIR}
  #   #LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
  #   #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # )

  # conda build
  install(TARGETS stidevicepy

    LIBRARY DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy/stidevicepy
    # On Windows with pybind11, a MODULE may be an import library + dll; this covers the .pyd
    RUNTIME DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy/stidevicepy
  )

  # conda build
  install(TARGETS stipybase

    LIBRARY DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy/stipybase
    # On Windows with pybind11, a MODULE may be an import library + dll; this covers the .pyd
    RUNTIME DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy/stipybase
  )
  # install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
  #         DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy
  #         # FILES_MATCHING PATTERN "*.py")
  # )
endif()

    
