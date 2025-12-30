cmake_minimum_required(VERSION 3.20)

# set the project name
project(profiles)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(sti3 CONFIG REQUIRED)


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

link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_executable(profiles main.cpp TestDevice.cpp)


target_link_libraries(profiles
  PRIVATE
    sti3::stidevice
    sti3::stinetwork
)
