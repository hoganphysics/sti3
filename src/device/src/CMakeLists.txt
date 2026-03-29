cmake_minimum_required(VERSION 3.14...3.20)

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


if(MSVC)
  add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()


# set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)  #sudo apt-get install libssl-dev

find_package(Boost 1.70 REQUIRED)


set(STI_BASE_DIR ../../..)

# include_directories("." ${Boost_INCLUDE_DIR})

add_library(stidevice SHARED 
    AppointmentTask.cpp
    AutoMonitor.cpp
    BinaryData.cpp
    CompressedStackTrace.cpp
    ConfigFile.cpp
    Configuration.cpp
    DeviceEventParser.cpp
    DeviceID.cpp
    DeviceMessage.cpp
    DeviceMessageReceiver.cpp
    EngineClock.cpp
    EngineID.cpp
    EngineJobSourceID.cpp
    EngineJobStatus.cpp
    EngineParsingMessage.cpp
    EngineParsingMessageCount.cpp
    EnginePlayingMessage.cpp
    EnginePlayingMessageCount.cpp
    EngineState.cpp
    EventEngineDependencyTree.cpp
    EventEngineManager.cpp
    EventEngineParser.cpp
    EventEngineStateMachine.cpp
    FileID.cpp
    FullShotResult.cpp
    Image.cpp
    IntervalTask.cpp
    LocalAttribute.cpp
    LocalAttributeManager.cpp
    LocalBinaryDataStream.cpp
    LocalChannel.cpp
    LocalChannelManager.cpp
    LocalMonitor.cpp
    LocalMonitorManager.cpp
    LocalDevice.cpp
    LocalDeviceMessageDispatcher.cpp
    LocalDeviceMessageHandler.cpp
    LocalEventEngine.cpp
    LocalEventEngineDependencyParser.cpp
    LocalEventEngineJob.cpp
    LocalEventEngineScheduler.cpp
    LocalFileHolder.cpp
    LocalFileServer.cpp
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
    PseudoSynchronousEvent.cpp
    LocalProfileManager.cpp
    RawEvent.cpp
    RawEventGroup.cpp
    RawEventTarget.cpp
    RawEventTargetChannel.cpp
    RawEventTargetDevice.cpp
    ResultTicket.cpp
    SerializedRepository.cpp
    Sequence.cpp
    SequenceID.cpp
    SequenceJob.cpp
    SequenceResult.cpp
    ShotConfig.cpp
    ShotID.cpp
    ShotResult.cpp
    ShotResultRecord.cpp
    StackTrace.cpp
    StackTraceData.cpp
    StackTraceResult.cpp
    STI_Exception.cpp
    SynchronousEvent.cpp
    Task.cpp
    TaskScheduler.cpp
    TimeStamp.cpp
    Ticket.cpp
    TransientRepository.cpp
    utils.cpp
    VirtualFileHolder.cpp
    VirtualFileServer.cpp
)

if(NOT TARGET sti3::stidevice)
  add_library(sti3::stidevice ALIAS stidevice)
endif()

if(MSVC)
# elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 13.0)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
  message("Using date.h")
  message("STI_BASE_DIR: ${STI_BASE_DIR}")
  # versions of gcc < 13 do not support std::chrono features required by AppointmentTask.cpp
  # using date.h instead
  find_package(CURL REQUIRED)
  target_sources(stidevice PRIVATE ${STI_BASE_DIR}/src/device/src/extern/date/tz.cpp)
  target_compile_definitions(stidevice PRIVATE USE_DATE_LIB=1)
  # target_link_libraries(stidevice PRIVATE ${CURL_LIBRARY})
  # message("CURL_LIBRARY: ${CURL_LIBRARY}")
  target_link_libraries(stidevice
    PRIVATE
      CURL::libcurl
  )
else()
  message(STATUS "Using native std::chrono (GCC >= 13 or non-GNU toolchain)")
endif()

target_link_libraries(stidevice
  PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::headers
)

set_target_properties(stidevice PROPERTIES 
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
)

set_target_properties(stidevice PROPERTIES
    VERSION ${PROJECT_VERSION}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    # MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

if(USE_STI_DEBUG_POSTFIX) 
  set_target_properties(stidevice PROPERTIES
    DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()


# target_include_directories(stidevice PRIVATE 
#     ${WORKING_DIRECTORY}
#     # ${OPENSSL_INCLUDE_DIR}
#     ${STI_BASE_DIR}/include
#     ${STI_BASE_DIR}/include/sti/extern
#     ${WORKING_DIRECTORY}/fwd
#     ${WORKING_DIRECTORY}/utils
    
# )

target_include_directories(stidevice 
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include> 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> 
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/sti/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/fwd
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# target_link_libraries(stidevice PRIVATE OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(stidevice 
#     ${OPENSSL_LIBRARIES}
# )

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
  # install(TARGETS stidevice
  #   #DESTINATION ${STI_BASE_DIR}/lib
  # LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
  #   #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # )

  # conda build
  install(TARGETS stidevice
    EXPORT sti3Targets 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(EXPORT sti3Targets
    NAMESPACE sti3::
    DESTINATION lib/cmake/sti3
  )
  # headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          )
          # FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.hxx")
  
  # if(MSVC)
  #   install(FILES
  #     $<TARGET_PDB_FILE:stidevice>
  #     DESTINATION ${CMAKE_INSTALL_BINDIR}  # or ${CMAKE_INSTALL_LIBDIR} or a subdir like debug/
  #   )
  # endif()

endif()
