cmake_minimum_required(VERSION 3.10)

# set the project name
project(STIServer VERSION 3.0.0 DESCRIPTION "STI Server" LANGUAGES CXX)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)


link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_executable(STIServer 
    main.cpp 
    ServerDevice.cpp
)
set_property(TARGET STIServer PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(STIServer PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/network/src
)

target_link_libraries(STIServer stidevice stinetwork)

install(TARGETS STIServer
    #DESTINATION ${STI_BASE_DIR}/lib
    DESTINATION ../../src/server/bin   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)