cmake_minimum_required(VERSION 3.20...3.22)

include(cmake/StiVersion.cmake)
include(cmake/StiBuildInfo.cmake)
sti3_load_version(${CMAKE_CURRENT_SOURCE_DIR})

project(sti3 VERSION ${STI3_VERSION} DESCRIPTION "sti3")

sti3_configure_build_info(
  ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  GENERATED_INCLUDE_DIR_VAR STI3_GENERATED_INCLUDE_DIR
)

set(STI_BASE_DIR ${sti3_SOURCE_DIR})
message("STI_BASE_DIR: ${STI_BASE_DIR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
option(STI3_BUILD_TESTS "Build STI3 unit tests" ON)

set(STI3_INSTALL_PDBS_DEFAULT OFF)
if(MSVC AND ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
  set(STI3_INSTALL_PDBS_DEFAULT ON)
endif()
option(STI3_INSTALL_PDBS "Install MSVC PDB symbol files" ${STI3_INSTALL_PDBS_DEFAULT})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Python COMPONENTS Interpreter Development REQUIRED)

# # Find pybind11 cmake directory
# execute_process(
#     COMMAND "${Python_EXECUTABLE}" -c
#         "import pybind11; print(pybind11.get_cmake_dir())"
#     OUTPUT_VARIABLE _tmp_dir
#     OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
# list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

add_subdirectory(src/device/src)
add_subdirectory(src/network/src)
add_subdirectory(src/server/src)
add_subdirectory(src/stipy/stidevicepy/src)
add_subdirectory(src/stipy/src)


if(STI3_BUILD_TESTS)
  include(CTest)          # enables BUILD_TESTING option + ctest integration
  enable_testing()
  add_subdirectory(test)
endif()


#install(TARGETS stipybase stidevicepy stipy
#    LIBRARY DESTINATION "src/stipy/bin"
#)


# Set these in the top-level CMakeLists, before targets:
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_BUILD_RPATH "")                       # optional: keep empty
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib;$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)      # <- key: don’t pull _build_env into RPATH



# ---- Export & package config ----

# Export all installed targets into one targets file + namespace
install(
  EXPORT sti3Targets
  NAMESPACE sti3::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sti3
)

# Minimal config that includes the targets file
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/conda/cmake/sti3Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sti3Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sti3
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/sti3ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/sti3Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/sti3ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sti3
)

install(
  DIRECTORY ${STI3_GENERATED_INCLUDE_DIR}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/examples/cpp/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/sti3/examples/cpp

  FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.ini"
    PATTERN "*.txt"
    PATTERN "*.cpp"
    PATTERN "*.json"
    PATTERN "CMakeLists.txt"
  
  # Exclude build directories and sti hidden files
  PATTERN "build" EXCLUDE
  PATTERN ".sti" EXCLUDE
)
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/examples/python/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/sti3/examples/python
  PATTERN "build" EXCLUDE
  PATTERN ".sti" EXCLUDE
  PATTERN "__pycache__" EXCLUDE
)
