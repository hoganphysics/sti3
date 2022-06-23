cmake_minimum_required(VERSION 3.10)

# set the project name
project(testDevice)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../../..)

if (MSVC)
set(USE_STI_DEBUG_POSTFIX True)
endif (MSVC)

if (USE_STI_DEBUG_POSTFIX)
set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)

link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_executable(testDevice main.cpp TestDevice.cpp)

target_include_directories(testDevice PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    #${STI_BASE_DIR}/src/network/src
)

target_link_libraries(testDevice stidevice${STI_LIB_POSTFIX} stinetwork${STI_LIB_POSTFIX})
