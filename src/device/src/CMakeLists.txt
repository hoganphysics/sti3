cmake_minimum_required(VERSION 3.10)

if (MSVC)
cmake_policy(SET CMP0091 NEW)
endif (MSVC)

# set the project name
project(stidevice VERSION 3.0.0 DESCRIPTION "STI device library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_DEBUG_POSTFIX "_d")
if (MSVC)
set(USE_STI_DEBUG_POSTFIX True)
endif (MSVC)


#set(STI_LIB_POSTFIX "$<$<CONFIG:Debug>:_d>")

if (USE_STI_DEBUG_POSTFIX)
#set(STI_LIB_POSTFIX "$<$<CONFIG:Debug>:_d>")
set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)

#set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
#set( CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1 )

#add_compile_definitions(_DEBUG)
if (MSVC)
add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif (MSVC)

find_package(OpenSSL REQUIRED)  #sudo apt-get install libssl-dev

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
    EngineClock.cpp
    EngineParsingMessage.cpp
    EventEngineDependencyTree.cpp
    EventEngineManager.cpp
    EventEngineParser.cpp
    EventEngineStateMachine.cpp
    EventStackTrace.cpp
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
    LocalFileHolder.cpp
    LocalPersistenceManager.cpp
    LocalResultsCollector.cpp
    MasterTrigger.cpp
    Measurement.cpp
    MetaData.cpp
    MixedValue.cpp
    ParseID.cpp
    ParsedDependencyTree.cpp
    ParseTicket.cpp  
    RawEvent.cpp
    ResultTicket.cpp
    SerializedRepository.cpp
    ShotID.cpp
    ShotResult.cpp
    SynchronousEvent.cpp
    TimeStamp.cpp
    Ticket.cpp
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
    # DEBUG_POSTFIX "_d"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#    DEBUG_POSTFIX ${STI_LIB_POSTFIX}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

if (USE_STI_DEBUG_POSTFIX) 
    set_target_properties(stidevice PROPERTIES
    #DEBUG_POSTFIX "_d"
    DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
    #DEBUG_POSTFIX "$<$<CONFIG:Debug>:_d>"
)
endif (USE_STI_DEBUG_POSTFIX)

target_include_directories(stidevice PRIVATE 
    ${WORKING_DIRECTORY}
    ${OPENSSL_INCLUDE_DIR}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/include/extern
    ${WORKING_DIRECTORY}/fwd
    ${WORKING_DIRECTORY}/utils
)

target_link_libraries(stidevice 
    ${OPENSSL_LIBRARIES}
)

install(TARGETS stidevice
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# target_link_libraries(testDevice stidevice stinetwork)
