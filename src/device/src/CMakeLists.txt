cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0091 NEW)

# set the project name
project(stidevice VERSION 3.0.0 DESCRIPTION "STI device library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_DEBUG_POSTFIX "_d")


#set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
#set( CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1 )

# add_compile_definitions(_DEBUG)

FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(STI_BASE_DIR ../../..)

include_directories("." ${Boost_INCLUDE_DIR})

#link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_library(stidevice SHARED 
    DeviceEventParser.cpp
    DeviceID.cpp
    DeviceMessage.cpp
    DeviceMessageReceiver.cpp
    # EngineParsingError.cpp
    EngineParsingMessage.cpp
    EventEngineDependencyTree.cpp
    EventEngineManager.cpp
    EventEngineParser.cpp
    EventEngineStateMachine.cpp
    EngineState.cpp
    LocalAttribute.cpp
    LocalAttributeManager.cpp
    LocalChannel.cpp
    LocalChannelManager.cpp
    LocalDevice.cpp
    LocalDeviceMessageDispatcher.cpp
    LocalDeviceMessageHandler.cpp
    LocalEventEngine.cpp
    LocalEventEngineJob.cpp
    LocalEventEngineScheduler.cpp
    MasterTrigger.cpp
    Measurement.cpp
    MetaData.cpp
    MixedValue.cpp
    RawEvent.cpp
    ShotID.cpp
    SynchronousEvent.cpp
    utils.cpp
)

set_target_properties(stidevice PROPERTIES 
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
)
# set_property(TARGET STIServer PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(stidevice PROPERTIES
    VERSION ${PROJECT_VERSION}
    DEBUG_POSTFIX "_d"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
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
