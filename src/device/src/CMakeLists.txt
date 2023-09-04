cmake_minimum_required(VERSION 3.14...3.20)

#if(MSVC)
#  cmake_policy(SET CMP0091 NEW)
#endif()

# set the project name
project(stidevice VERSION 3.0.0 DESCRIPTION "STI device library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)



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


if(USE_STI_DEBUG_POSTFIX)
  set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)

#set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
#set( CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1 )

#add_compile_definitions(_DEBUG)
if(MSVC)
  add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()


set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)  #sudo apt-get install libssl-dev

find_package(Boost 1.64 REQUIRED)



set(STI_BASE_DIR ../../..)

include_directories("." ${Boost_INCLUDE_DIR})

#link_directories(${STI_BASE_DIR}/lib)

add_library(stidevice SHARED 
    AppointmentTask.cpp
    BinaryData.cpp
    ConfigFile.cpp
    Configuration.cpp
    DefaultImageWriter.cpp
    DeviceEventParser.cpp
    DeviceID.cpp
    DeviceMessage.cpp
    DeviceMessageReceiver.cpp
    # EngineParsingError.cpp
    EngineClock.cpp
    EngineID.cpp
    EngineJobSourceID.cpp
    EngineParsingMessage.cpp
    EngineState.cpp
    EventEngineDependencyTree.cpp
    EventEngineManager.cpp
    EventEngineParser.cpp
    EventEngineStateMachine.cpp
    FullShotResult.cpp
    # EngineParseResult.cpp
    Image.cpp
    IntervalTask.cpp
    LocalAttribute.cpp
    LocalAttributeManager.cpp
    LocalChannel.cpp
    LocalChannelManager.cpp
    LocalDevice.cpp
    LocalDeviceMessageDispatcher.cpp
    LocalDeviceMessageHandler.cpp
    LocalEventEngine.cpp
    LocalEventEngineDependencyParser.cpp
    LocalEventEngineJob.cpp
    LocalEventEngineScheduler.cpp
    LocalFileHolder.cpp
    LocalPersistenceManager.cpp
    LocalResultsCollector.cpp
    LocalShot.cpp
    LocalLogManager.cpp
    LocalTaskManager.cpp
    Logger.cpp
    LogID.cpp
    LogRecord.cpp
    LogRecordFile.cpp
    MasterTrigger.cpp
    Measurement.cpp
    MetaData.cpp
    MixedValue.cpp
    ParsedDependencyTree.cpp
    ParsedTag.cpp
    ParsedVar.cpp
    ParseID.cpp
    ParseResult.cpp
    ParseTicket.cpp  
    PartnerDevice.cpp
    PersistenceTargetHolder.cpp
    Profile.cpp
    LocalProfileManager.cpp
    RawEvent.cpp
    RawEventGroup.cpp
    # RawEventGroupManager.cpp
    RawEventTarget.cpp
    RawEventTargetChannel.cpp
    RawEventTargetDevice.cpp
    RawStackTrace.cpp
    ResultTicket.cpp
    SerializedRepository.cpp
    Sequence.cpp
    SequenceID.cpp
    SequenceResult.cpp
    ShotConfig.cpp
    ShotID.cpp
    ShotResult.cpp
    ShotResultRecord.cpp
    StackTrace.cpp
    StackTraceData.cpp
    StackTraceResult.cpp
    SynchronousEvent.cpp
    Task.cpp
    TaskScheduler.cpp
    TimeStamp.cpp
    Ticket.cpp
    TransientRepository.cpp
    utils.cpp
)

#set_property(TARGET tgt PROPERTY CXX_STANDARD 11)
#set_target_properties(stidevice PROPERTIES CXX_STANDARD 23)

if(MSVC)
else()
  message("Using date.h")
  message("STI_BASE_DIR: ${STI_BASE_DIR}")
  # versions of gcc < 13 do not support std::chrono features required by AppointmentTask.cpp
  # using date.h instead
  find_package(CURL REQUIRED)
  target_sources(stidevice PRIVATE ${STI_BASE_DIR}/src/device/src/extern/date/tz.cpp)
  target_compile_definitions(stidevice PRIVATE USE_DATE_LIB=1)
  target_link_libraries(stidevice ${CURL_LIBRARY})
  message("CURL_LIBRARY: ${CURL_LIBRARY}")
endif()

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
)

if(USE_STI_DEBUG_POSTFIX) 
  set_target_properties(stidevice PROPERTIES
    DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()


target_include_directories(stidevice PRIVATE 
    ${WORKING_DIRECTORY}
    ${OPENSSL_INCLUDE_DIR}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/include/sti/extern
    ${WORKING_DIRECTORY}/fwd
    ${WORKING_DIRECTORY}/utils
)

target_link_libraries(stidevice 
    ${OPENSSL_LIBRARIES}
)

if(DEFINED SKBUILD)
  if(MSVC)
    install(TARGETS stidevice
      RUNTIME DESTINATION src/stipy/bin
    )
  else()
    install(TARGETS stidevice
      LIBRARY DESTINATION src/stipy/bin
    )
  endif()
else()
  install(TARGETS stidevice
    #DESTINATION ${STI_BASE_DIR}/lib
  LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()
# target_link_libraries(testDevice stidevice stinetwork)
