cmake_minimum_required(VERSION 3.10)

if (MSVC)
cmake_policy(SET CMP0091 NEW)
endif (MSVC)

# set the project name
project(stinetwork VERSION 3.0.0 DESCRIPTION "STI network library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
set(CMAKE_DEBUG_POSTFIX "_d")
endif (MSVC)

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )
find_package(Threads REQUIRED)

#find_package(OmniORB 4.2.1 COMPONENTS program_options REQUIRED)
if (MSVC)
add_compile_definitions(__WIN32__ __x86__ _WIN32_WINNT=0x0400 __NT__ __OSVERSION__=4 _WINDOWS)   #_DEBUG  NDEBUG
endif (MSVC)

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
    COSBindingNode.cpp
    deviceNet.cpp
    deviceNetDynSK.cpp
    NetworkConvert.cpp
    NetworkDeviceHub.cpp
    NetworkDeviceHubWrapper.cpp
    NetworkDeviceMessageHandlerWrapper.cpp
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
    RemoteEventEngineScheduler.cpp
    RemoteShot.cpp
    RemoteTriggerCallback.cpp
    TAttributeManager_i.cpp
    TChannelManager_i.cpp
    TDeviceCollection_i.cpp
    TDeviceHub_i.cpp
    TDevice_i.cpp
    TDeviceMessageDispatcher_i.cpp
    TDeviceMessageHandler_i.cpp
    TEventEngine_i.cpp
    TEventEngineScheduler_i.cpp
    TRefreshIndicator_i.cpp
    TShot_i.cpp
    TTriggerCallback_i.cpp
)

if (MSVC)

    target_link_directories(stinetwork PUBLIC 
        "${OMNIORB_PATH}/lib/x86_win32"
        "${STI_BASE_DIR}/lib"
    # "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Debug/lib"
    # "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Release/lib"
    )

    set(STIDEVICE_LIB stidevice${CMAKE_DEBUG_POSTFIX})
    set(OMNI_EXT "_rt$<$<CONFIG:Debug>:d>")

endif (MSVC)

# if (MSVC)
#     set(OMNI_EXT "_rt$<$<CONFIG:Debug>:d>")
# endif (MSVC)


target_link_libraries(stinetwork 
    Threads::Threads
    # pthread
    omnithread${OMNI_EXT}
    omniORB4${OMNI_EXT}
    omniDynamic4${OMNI_EXT}
    omniCodeSets4${OMNI_EXT}
    # omnisslTP4   #if OpenSSL is available
    COS4${OMNI_EXT}
    COSDynamic4${OMNI_EXT}
    ${STIDEVICE_LIB}
)

#set_target_properties(stinetwork PROPERTIES VERSION ${PROJECT_VERSION})

#https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows



set_target_properties(stinetwork PROPERTIES
    VERSION ${PROJECT_VERSION}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1
    BUILD_SHARED_LIBS TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    #MSVC_RUNTIME_LIBRARY "MultiThreadedDebug>"
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

message("OMNIORB_PATH:")
message(STATUS "${OMNIORB_PATH}")


#file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "CONFIG is $<CONFIG>")
message(STATUS "${CMAKE_CURRENT_BINARY_DIR}")
#file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "MSVC_RUNTIME_LIBRARY is MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt CONTENT "OMNI_EXT is ${OMNI_EXT}")


target_include_directories(stinetwork PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    # ${WORKING_DIRECTORY}/fwd
    # ${WORKING_DIRECTORY}/utils
)

install(TARGETS stinetwork
    #DESTINATION ${STI_BASE_DIR}/lib
    LIBRARY DESTINATION ../../lib   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# target_link_libraries(testDevice stidevice stinetwork)
