cmake_minimum_required(VERSION 3.10)

# set the project name
project(testConfig)

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
add_executable(testConfig main.cpp)

target_include_directories(testConfig PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
)

target_link_libraries(testConfig stidevice${STI_LIB_POSTFIX})
