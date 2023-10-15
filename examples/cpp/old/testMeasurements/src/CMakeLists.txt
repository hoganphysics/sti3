cmake_minimum_required(VERSION 3.10)

# set the project name
project(testMeasurements)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../../..)


link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_executable(testMeasurements main.cpp)

target_include_directories(testMeasurements PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
)

target_link_libraries(testMeasurements stidevice stinetwork)

install(TARGETS testMeasurements
    DESTINATION .
    # DESTINATION ../../src/server/bin   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
)
