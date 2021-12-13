cmake_minimum_required(VERSION 3.10)

# set the project name
project(stijava VERSION 3.0.0 DESCRIPTION "STI Java wrapper library")

find_package(JNI REQUIRED)

if (JNI_FOUND)
    message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
    message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()


# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../../..)

include_directories(".")
# include_directories("${STI_BASE_DIR}/src/device/src")

link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_library(stijava SHARED 
    JAttributeManager.cpp
    JChannelManager.cpp
    JDeviceCollection.cpp
    JDevice.cpp
    JDeviceMessageDispatcher.cpp
    JDeviceMessageReceiver.cpp
    JEventEngine.cpp
    JEventEngineJob.cpp
    JEventEngineScheduler.cpp
    JLocalDevice.cpp
    JNetworkDeviceHub.cpp
    JNodeWalker.cpp
    JPersistenceManager.cpp
    JShot.cpp
    sti_wrap.cpp
)


target_link_libraries(stijava 
    stidevice
    stinetwork
)

#set_target_properties(stinetwork PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stijava PROPERTIES
    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(stijava PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/src/device/src/utils
    ${STI_BASE_DIR}/src/network/src
    ${JNI_INCLUDE_DIRS}
    # ${WORKING_DIRECTORY}/fwd
    # ${WORKING_DIRECTORY}/utils
)

install(TARGETS stijava
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# target_link_libraries(stijava stidevice stinetwork)
