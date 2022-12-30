cmake_minimum_required(VERSION 3.10...3.20)

#if (MSVC)
#  cmake_policy(SET CMP0091 NEW)
#endif()

# set the project name
project(stinetwork VERSION 3.0.0 DESCRIPTION "STI network library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
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


#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )
find_package(Threads REQUIRED)

#find_package(OmniORB 4.2.1 COMPONENTS program_options REQUIRED)
if (MSVC)
  add_compile_definitions(__WIN32__ __x86__ _WIN32_WINNT=0x0400 __NT__ __OSVERSION__=4 _WINDOWS)   #_DEBUG  NDEBUG
  add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()



set(STI_BASE_DIR ../../..)

# include_directories("." ${OMNIORB4_INCLUDE_DIR})
#include_directories("." "C:\Users\Jason\Code\lib\omniORB-4.2.1.x64\include")
include_directories("." ${Boost_INCLUDE_DIR} "${OMNIORB_PATH}/include")

# include_directories("${STI_BASE_DIR}/src/device/src")

#link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_library(stinetwork SHARED 
    Convert_Attribute.cpp
    Convert_Channel.cpp
    Convert_DeviceID.cpp
    Convert_DeviceMessage.cpp
    Convert_EventEngine.cpp
    Convert_HubID.cpp
    Convert_HubNodeWalker.cpp
    Convert_HubTrace.cpp
    Convert_DeviceTrace.cpp
    Convert_RawEventGroup.cpp
    Convert_ResultsCollector.cpp
    Convert_SequenceResult.cpp
    Convert_ShotResult.cpp
    Convert_StackTrace.cpp
    #Convert_ResultTicket.cpp
    COSBindingNode.cpp
    deviceNet.cpp
    deviceNetDynSK.cpp
    NetworkConvert.cpp
    NetworkDeviceHub.cpp
    NetworkDeviceHubWrapper.cpp
    NetworkDeviceWrapper.cpp
    NetworkDeviceMessageHandlerWrapper.cpp
    NetworkEventEngine.cpp
    NetworkFileHolder.cpp
    NetworkShotWrapper.cpp
    NetworkResultsCollector.cpp
    #NetworkShotRepositoryWrapper.cpp
    ORBManager.cpp
    orbTypes.cpp
    orbTypesDynSK.cpp
    RemoteAttribute.cpp
    RemoteAttributeManager.cpp
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
    RemotePersistenceManager.cpp
    RemoteResultsCollector.cpp
    RemoteShot.cpp
    #RemoteShotRepository.cpp
    RemoteTriggerCallback.cpp
    TAttributeManager_i.cpp
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
    TPersistenceManager_i.cpp
    TRefreshIndicator_i.cpp
    TResultsCollector_i.cpp
    TShotCallback_i.cpp
    #TShotRepository_i.cpp
    TTriggerCallback_i.cpp
)

add_dependencies(stinetwork stidevice)

if(MSVC)

    target_link_directories(stinetwork PUBLIC 
        "${OMNIORB_PATH}/lib/x86_win32"
        #"${STI_BASE_DIR}/lib"
        #"C:/Users/Jason/Code/dev/sti3/build/x64-Debug/src/device/src"
        #"src/device/src"
        "${CMAKE_CURRENT_BINARY_DIR}/../../device/src"
    # "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Debug/lib"
    # "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Release/lib"
    )

    # set(STIDEVICE_LIB stidevice${CMAKE_DEBUG_POSTFIX})
    set(OMNI_EXT "_rt$<$<CONFIG:Debug>:d>")

endif()

# if (MSVC)
#     set(OMNI_EXT "_rt$<$<CONFIG:Debug>:d>")
# endif (MSVC)


target_link_libraries(stinetwork 
    Threads::Threads
    # pthread
    omnithread${OMNI_EXT}
    omniORB4${OMNI_EXT}
    omniDynamic4${OMNI_EXT}
    #omniCodeSets4${OMNI_EXT}
    # omnisslTP4   #if OpenSSL is available
    #COS4${OMNI_EXT}
    #COSDynamic4${OMNI_EXT}
    # stidevice${STI_LIB_POSTFIX}
    # Ws2_32
)



if(MSVC)
    target_link_libraries(stinetwork 
        stidevice${STI_LIB_POSTFIX}
    )
endif()

#set_target_properties(stinetwork PROPERTIES VERSION ${PROJECT_VERSION})

#https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows





set_target_properties(stinetwork PROPERTIES
    VERSION ${PROJECT_VERSION}
#    DEBUG_POSTFIX "$<IF:$<CONFIG:Debug>,${STI_LIB_POSTFIX},"">"
#    "$<$<CONFIG:Debug>,DEBUG_POSTFIX ${STI_LIB_POSTFIX}>"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    #MSVC_RUNTIME_LIBRARY "MultiThreadedDebug>"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

if(USE_STI_DEBUG_POSTFIX) 
  set_target_properties(stinetwork PROPERTIES
    DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
  )
endif()

#if (USE_STI_DEBUG_POSTFIX) 
#    set_target_properties(stinetwork PROPERTIES
#        DEBUG_POSTFIX ${STI_LIB_POSTFIX} 
#    )
#endif (USE_STI_DEBUG_POSTFIX)


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

get_omni_msvc(OMNI_MSVC_VER)
message("Output: ${OMNI_MSVC_VER}")

message("Lib: omniORB4_${OMNI_MSVC_VER}${OMNI_EXT}${CMAKE_SHARED_LIBRARY_SUFFIX}")

target_include_directories(stinetwork PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    ${STI_BASE_DIR}/include/sti/extern
    # ${WORKING_DIRECTORY}/fwd
    # ${WORKING_DIRECTORY}/utils
)

if(MSVC)
    target_include_directories(stinetwork PUBLIC 
        ${OMNIORB_PATH}/include
    )
endif()

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
  message(">>>>>> STI Network: regular CMake build")
  install(TARGETS stinetwork
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()



# target_link_libraries(testDevice stidevice stinetwork)
