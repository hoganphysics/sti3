cmake_minimum_required(VERSION 3.10)

# set the project name
project(testDevice)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../../..)


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
add_executable(testDevice main.cpp TestDevice.cpp)

target_include_directories(testDevice PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
)

target_link_libraries(testDevice stidevice${STI_LIB_POSTFIX} stinetwork${STI_LIB_POSTFIX})
