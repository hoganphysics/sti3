cmake_minimum_required(VERSION 3.10...3.20)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endif()

# set the project name
project(stipy VERSION ${STI3_VERSION} DESCRIPTION "STIPy python wrapper" LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)


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


if(USE_STI_DEBUG_POSTFIX)
  set(STI_LIB_POSTFIX "_d")
endif()

include_directories(".")

if(MSVC)
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



find_package(pybind11 REQUIRED)
message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")


pybind11_add_module(stipy 
    ParseTicket_wrap.cpp
    ParsedTag_wrap.cpp
    ParsedVar_wrap.cpp
    pywrapper.cpp    
    PyParseTicket.cpp
    PyResultTicket.cpp    
    ResultTicket_wrap.cpp
    stipy.cpp
    STIPyGlobal.cpp
    STIPyLibDevice.cpp
    STIPyServer.cpp
    STIPyServer_wrap.cpp
    STIPyShot.cpp
    STIPyShot_wrap.cpp
    stipy_wrap.cpp
)

add_dependencies(stipy 
                sticommonpy stidevice stinetwork)

if(MSVC)
    target_link_directories(stipy PUBLIC 
        "${CMAKE_CURRENT_BINARY_DIR}/../stidevicepy/src"
        "${CMAKE_CURRENT_BINARY_DIR}/../../device/src"
        "${CMAKE_CURRENT_BINARY_DIR}/../../network/src"
    )
endif()

set_target_properties(stipy PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    # MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

target_compile_definitions(stipy PRIVATE
  PYBIND11_DETAILED_ERROR_MESSAGES=1
  PYBIND11_ASSERT_GIL_HELD_INCREF_DECREF=1  # optional, catches GIL mistakes
)


#https://github.com/pybind/cmake_example/issues/11
if(UNIX AND NOT APPLE)
    set_target_properties(stipy PROPERTIES INSTALL_RPATH "$ORIGIN")
    message("Setting RPATH in UNIX")
elseif(APPLE)
    set_target_properties(stipy PROPERTIES INSTALL_RPATH "@loader_path")
endif()


target_include_directories(stipy PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/stipy/stidevicepy/src
    ${STI_BASE_DIR}/src/network/src
)

if(MSVC)
  set(STIDEVICEPY_INSTALL_DIR "bin")
else()
  set(STIDEVICEPY_INSTALL_DIR "../../lib")
endif()


target_link_libraries(stipy PRIVATE 
    stidevice${STI_LIB_POSTFIX}
    stinetwork${STI_LIB_POSTFIX}
    sticommonpy${STI_LIB_POSTFIX}
)

if(DEFINED SKBUILD)
  install(TARGETS stipy
    LIBRARY DESTINATION src/stipy/bin
  )
else()
  # install(TARGETS stipy
  #   #DESTINATION ${STI_BASE_DIR}/lib
  #   LIBRARY DESTINATION ${STIDEVICEPY_INSTALL_DIR}
  #   #LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
  #   #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # )

  # Ensure CMAKE_INSTALL_PYTHONDIR is CMake-style (forward slashes)
  file(TO_CMAKE_PATH "${CMAKE_INSTALL_PYTHONDIR}" CMAKE_INSTALL_PYTHONDIR)

  # conda build
  install(TARGETS stipy
    LIBRARY DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy
    # On Windows with pybind11, a MODULE may be an import library + dll; this covers the .pyd
    RUNTIME DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy
  )
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
          DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/stipy
          FILES_MATCHING PATTERN "*.py")
  
endif()

