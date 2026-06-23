cmake_minimum_required(VERSION 3.10...3.20)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endif()

# set the project name
project(stinetwork VERSION ${STI3_VERSION} DESCRIPTION "STI network library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)  # version 20 causes compile error with omniORB in gcc build
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


if (USE_STI_DEBUG_POSTFIX)
  set(STI_LIB_POSTFIX "_d")
endif (USE_STI_DEBUG_POSTFIX)

message("STI_LIB_POSTFIX:")
message(STATUS "${STI_LIB_POSTFIX}")


find_package(Boost 1.70 REQUIRED)
find_package(Threads REQUIRED)
# find_package(OmniORB 4.3.0 COMPONENTS program_options REQUIRED)
find_package(omniORB CONFIG QUIET)

if (MSVC)
  add_compile_definitions(__WIN32__ __x86__ _WIN32_WINNT=0x0400 __NT__ __OSVERSION__=4 _WINDOWS)   #_DEBUG  NDEBUG
  add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()

if(NOT TARGET sti3::stidevice)
  find_package(sti3 CONFIG REQUIRED)
endif()

# if (DEFINED ENV{OMNIORB_ROOT_DIR})
#     message("Enviromental var defined!!")
#     set(OMNIORB_PATH $ENV{OMNIORB_ROOT_DIR})
# else()
#     message("Enviromental NOT defined")
#     set(OMNIORB_PATH "")
# endif()

set(STI_BASE_DIR ../../..)

# include_directories("." ${Boost_INCLUDE_DIR} "${OMNIORB_PATH}/include")


add_library(stinetwork SHARED 
    convert/Convert_Log.cpp
    convert/Convert_Profile.cpp
    convert/Convert_Attribute.cpp
    convert/Convert_Channel.cpp
    convert/Convert_DeviceID.cpp
    convert/Convert_DeviceMessage.cpp
    convert/Convert_EventEngine.cpp
    convert/Convert_PostProcessing.cpp
    convert/Convert_File.cpp
    convert/Convert_HubID.cpp
    convert/Convert_HubNodeWalker.cpp
    convert/Convert_HubTrace.cpp
    convert/Convert_Log.cpp
    convert/Convert_Monitor.cpp
    convert/Convert_Profile.cpp
    convert/Convert_DeviceTrace.cpp
    convert/Convert_RawEventGroup.cpp
    convert/Convert_ResultsCollector.cpp
    convert/Convert_SequenceResult.cpp
    convert/Convert_ShotResult.cpp
    convert/Convert_StackTrace.cpp
    convert/Convert_Task.cpp
    convert/Convert_VersionInfo.cpp

    generated/deviceNet.cpp
    generated/deviceNetDynSK.cpp
    generated/logsNet.cpp
    generated/logsNetDynSK.cpp
    generated/orbTypes.cpp
    generated/orbTypesDynSK.cpp
    generated/tasks.cpp
    generated/tasksDynSK.cpp

    COSBindingNode.cpp
    NetworkBinaryDataStream.cpp
    NetworkBinaryDataStreamTarget.cpp
    NetworkConvert.cpp
    NetworkDeviceHub.cpp
    NetworkDeviceHubWrapper.cpp
    NetworkDevice.cpp
    NetworkDeviceMessageHandlerWrapper.cpp
    NetworkEventEngine.cpp
    NetworkFileHolder.cpp
    NetworkFileServer.cpp
    NetworkShotWrapper.cpp
    NetworkResultsCollector.cpp
    ORBManager.cpp

    RemoteAttribute.cpp
    RemoteAttributeManager.cpp
    RemoteBinaryDataStream.cpp
    RemoteBinaryDataStreamTarget.cpp
    RemoteChannel.cpp
    RemoteChannelManager.cpp
    RemoteDeviceCollection.cpp
    RemoteDevice.cpp
    RemoteDeviceHub.cpp
    RemoteDeviceMessageDispatcher.cpp
    RemoteDeviceMessageHandler.cpp
    RemoteEventEngine.cpp
    RemoteEventEngineDependencyParser.cpp
    RemoteEventEngineScheduler.cpp
    RemoteFileHolder.cpp
    RemoteFileServer.cpp
    RemoteLogManager.cpp
    RemoteMonitor.cpp
    RemoteMonitorManager.cpp
    RemotePersistenceManager.cpp
    RemotePostProcessingManager.cpp
    RemoteProfileManager.cpp
    RemoteResultsCollector.cpp
    RemoteTask.cpp
    RemoteTaskManager.cpp
    RemoteShot.cpp
    RemoteTriggerCallback.cpp
    RemoteVersionManager.cpp

    TAttributeManager_i.cpp
    TBinaryDataStream_i.cpp
    TBinaryDataStreamTarget_i.cpp
    TChannelManager_i.cpp
    TDeviceCollection_i.cpp
    TDeviceHub_i.cpp
    TDevice_i.cpp
    TDeviceMessageDispatcher_i.cpp
    TDeviceMessageHandler_i.cpp
    TEventEngine_i.cpp
    TEventEngineDependencyParser_i.cpp
    TEventEngineScheduler_i.cpp
    TFileHolder_i.cpp
    TFileServer_i.cpp
    TLogManager_i.cpp
    TMonitorManager_i.cpp
    TPersistenceManager_i.cpp
    TPostProcessingManager_i.cpp
    TProfileManager_i.cpp
    TRefreshIndicator_i.cpp
    TResultsCollector_i.cpp
    TShotCallback_i.cpp
    TTaskManager_i.cpp
    TTestNetwork_i.cpp
    TestNetworkWrapper.cpp
    TTriggerCallback_i.cpp
)

if(NOT TARGET sti3::stinetwork)
  add_library(sti3::stinetwork ALIAS stinetwork)
endif()

# add_dependencies(stinetwork stidevice)

# if(MSVC)
#     target_link_directories(stinetwork PUBLIC 
#         "${OMNIORB_PATH}/lib/x86_win32"
#         #"${STI_BASE_DIR}/lib"
#         "${CMAKE_CURRENT_BINARY_DIR}/../../device/src"
#     )
#     set(OMNI_EXT "_rt$<$<CONFIG:Debug>:d>")
# endif()


# If no imported targets were provided, create them by hand
if(NOT TARGET OmniORB::omnithread)
  # Choose the conda prefix: $PREFIX inside conda-build, or $CONDA_PREFIX when testing locally
  set(_CONDAPREFIX $ENV{PREFIX})
  if(NOT _CONDAPREFIX)
    set(_CONDAPREFIX $ENV{CONDA_PREFIX})
  endif()

  if(WIN32)
    set(_inc_hints "${_CONDAPREFIX}/Library/include" "${_CONDAPREFIX}/include")
    set(_lib_hints "${_CONDAPREFIX}/Library/lib"     "${_CONDAPREFIX}/lib")
    set(_names_omnithread  omnithread omnithread_rt omnithread4)
    set(_names_omniORB4    omniORB4   omniORB4_rt)
    set(_names_omniDynamic4 omniDynamic4 omniDynamic4_rt)
  else()
    set(_inc_hints "${_CONDAPREFIX}/include")
    set(_lib_hints "${_CONDAPREFIX}/lib")
    set(_names_omnithread  omnithread)
    set(_names_omniORB4    omniORB4)
    set(_names_omniDynamic4 omniDynamic4)
  endif()

  find_path(OMNIORB_INCLUDE_DIR
    NAMES omniORB4/CORBA.h
    HINTS ${_inc_hints}
  )
  find_library(OMNIORB_LIB     NAMES ${_names_omniORB4}    HINTS ${_lib_hints})
  find_library(OMNITHREAD_LIB  NAMES ${_names_omnithread}  HINTS ${_lib_hints})
  find_library(OMNIDYNAMIC_LIB NAMES ${_names_omniDynamic4} HINTS ${_lib_hints})

  if(NOT OMNIORB_INCLUDE_DIR OR NOT OMNIORB_LIB OR NOT OMNITHREAD_LIB OR NOT OMNIDYNAMIC_LIB)
    message(FATAL_ERROR
      "omniORB not found in conda prefix '${_CONDAPREFIX}'.\n"
      "  Tried includes: ${_inc_hints}\n"
      "  Tried libs:     ${_lib_hints}")
  endif()

  add_library(OmniORB::omniORB4 UNKNOWN IMPORTED)
  set_target_properties(OmniORB::omniORB4 PROPERTIES
    IMPORTED_LOCATION "${OMNIORB_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${OMNIORB_INCLUDE_DIR}"
  )
  add_library(OmniORB::omnithread UNKNOWN IMPORTED)
  set_target_properties(OmniORB::omnithread PROPERTIES
    IMPORTED_LOCATION "${OMNITHREAD_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${OMNIORB_INCLUDE_DIR}"
  )
  add_library(OmniORB::omniDynamic4 UNKNOWN IMPORTED)
  set_target_properties(OmniORB::omniDynamic4 PROPERTIES
    IMPORTED_LOCATION "${OMNIDYNAMIC_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${OMNIORB_INCLUDE_DIR}"
  )
endif()

target_link_libraries(stinetwork
  PUBLIC
    sti3::stidevice
    # stidevice
  PRIVATE
    Threads::Threads
    Boost::headers
    OmniORB::omnithread
    OmniORB::omniORB4
    OmniORB::omniDynamic4
)

# target_link_libraries(stinetwork PUBLIC sti3::stidevice)


get_target_property(_links stinetwork LINK_LIBRARIES)
message(STATUS "stinetwork LINK_LIBRARIES=${_links}")


# target_link_libraries(stinetwork
#   PRIVATE
#     Threads::Threads
#     Boost::headers
#     # pthread
#     # omnithread${OMNI_EXT}
#     # omniORB4${OMNI_EXT}
#     # omniDynamic4${OMNI_EXT}
#     # OmniORB::omnithread
#     # OmniORB::omniORB4
#     # OmniORB::omniDynamic4
# )



# if(MSVC)
#     target_link_libraries(stinetwork 
#       PUBLIC
#         stidevice${STI_LIB_POSTFIX}
#     )
# endif()

#set_target_properties(stinetwork PROPERTIES VERSION ${PROJECT_VERSION})

#https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows


set_target_properties(stinetwork PROPERTIES
    VERSION ${PROJECT_VERSION}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    # MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

if(USE_STI_DEBUG_POSTFIX) 
  set_target_properties(stinetwork PROPERTIES
    DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()


message("OMNIORB_PATH:")
message(STATUS "${OMNIORB_PATH}")


#file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "CONFIG is $<CONFIG>")
message(STATUS "${CMAKE_CURRENT_BINARY_DIR}")
#file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "MSVC_RUNTIME_LIBRARY is MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
#file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "OMNI_EXT is ${OMNI_EXT}")
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "MSVC_TOOLSET_VERSION is ${MSVC_TOOLSET_VERSION}  MSVC_VERSION  is ${MSVC_VERSION}  CMAKE_CXX_COMPILER_VERSION  is ${CMAKE_CXX_COMPILER_VERSION}")
#MSVC_TOOLSET_VERSION
#omnithread${OMNI_EXT}.${CMAKE_SHARED_LIBRARY_SUFFIX}


function(GET_OMNI_MSVC REQUIRED_ARG)
  if(    "${MSVC_TOOLSET_VERSION}" STREQUAL "80")
    set(${REQUIRED_ARG} "vc8" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "90")
    set(${REQUIRED_ARG} "vc9" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "100")
    set(${REQUIRED_ARG} "vc10" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "110")
    set(${REQUIRED_ARG} "vc11" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "120")
    set(${REQUIRED_ARG} "vc12" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "140")
    set(${REQUIRED_ARG} "vc14" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "141")
    set(${REQUIRED_ARG} "vc15" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "142")
    set(${REQUIRED_ARG} "vc16" PARENT_SCOPE)
  elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "143")
    set(${REQUIRED_ARG} "vc17" PARENT_SCOPE)
  else()
    set(${REQUIRED_ARG} "unknown" PARENT_SCOPE)
  endif()
endfunction()

# Lookup MSVC version code for omniORB
get_omni_msvc(OMNI_MSVC_VER)
message("Output: ${OMNI_MSVC_VER}")

message("Lib: omniORB4_${OMNI_MSVC_VER}${OMNI_EXT}${CMAKE_SHARED_LIBRARY_SUFFIX}")

# target_include_directories(stinetwork PRIVATE 
#     ${WORKING_DIRECTORY}
#     ${STI_BASE_DIR}/include
#     ${STI_BASE_DIR}/src/device/src
#     ${STI_BASE_DIR}/src/network/src/generated
#     ${STI_BASE_DIR}/include/sti/extern
# )


target_include_directories(stinetwork PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/device/src
    ${CMAKE_CURRENT_SOURCE_DIR}/convert
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/sti/extern
)


# if(MSVC)
#     target_include_directories(stinetwork PUBLIC 
#         ${OMNIORB_PATH}/include
#     )
# endif()

if(DEFINED SKBUILD)
  message(">>>>>> STI Network: SKBUILD")
  if(MSVC)
    install(TARGETS stinetwork
      RUNTIME DESTINATION src/stipy/bin
    )
  else()
    install(TARGETS stinetwork
      LIBRARY DESTINATION src/stipy/bin
    )
  endif()
else()
  # message(">>>>>> STI Network: regular CMake build")
  # install(TARGETS stinetwork
  #   #DESTINATION ${STI_BASE_DIR}/lib
  #   LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
  #   #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # )

  # conda build
  install(TARGETS stinetwork
    EXPORT sti3Targets                 # <-- put targets into this export set
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  # headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          )

  if(MSVC AND STI3_INSTALL_PDBS)
    install(FILES
      $<TARGET_PDB_FILE:stinetwork>
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      OPTIONAL
    )
  endif()
endif()
