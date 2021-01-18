cmake_minimum_required(VERSION 3.10)

# set the project name
project(stidevice VERSION 3.0.0 DESCRIPTION "STI device library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)

include_directories(".")

#link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_library(stidevice SHARED 
    DeviceEventParser.cpp
    DeviceID.cpp
    DeviceMessage.cpp
    DeviceMessageReceiver.cpp
    EngineParsingError.cpp
    EventEngineDependencyTree.cpp
    EventEngineManager.cpp
    EventEngineParser.cpp
    EventEngineStateMachine.cpp
    LocalChannel.cpp
    LocalChannelManager.cpp
    LocalDevice.cpp
    LocalDeviceMessageDispatcher.cpp
    LocalDeviceMessageHandler.cpp
    LocalEventEngine.cpp
    LocalEventEngineJob.cpp
    LocalEventEngineScheduler.cpp
    Measurement.cpp
    MixedValue.cpp
    RawEvent.cpp
    SynchronousEvent.cpp
    utils.cpp
)

set_target_properties(stidevice PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stidevice PROPERTIES
    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

target_include_directories(stidevice PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${WORKING_DIRECTORY}/fwd
    ${WORKING_DIRECTORY}/utils
)

install(TARGETS stidevice
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# target_link_libraries(testDevice stidevice stinetwork)
