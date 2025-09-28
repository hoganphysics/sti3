cmake_minimum_required(VERSION 3.20...3.22)
project(sti3 VERSION 3.0.1 DESCRIPTION "sti3")

set(STI_BASE_DIR ${sti3_SOURCE_DIR})
message("STI_BASE_DIR: ${STI_BASE_DIR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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