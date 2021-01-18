cmake_minimum_required(VERSION 3.10)

# set the project name
project(stinetwork VERSION 3.0.0 DESCRIPTION "STI network library")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(STI_BASE_DIR ../../..)

include_directories(".")
# include_directories("${STI_BASE_DIR}/src/device/src")

#link_directories(${STI_BASE_DIR}/lib)

# add the executable
add_library(stinetwork SHARED 
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
    RemoteChannelManager.cpp
    RemoteDeviceCollection.cpp
    RemoteDevice.cpp
    RemoteDeviceHub.cpp
    RemoteDeviceMessageDispatcher.cpp
    RemoteDeviceMessageHandler.cpp
    RemoteEventEngine.cpp
    RemoteEventEngineScheduler.cpp
    RemoteParsedShot.cpp
    RemoteTriggerCallback.cpp
    TChannelManager_i.cpp
    TDeviceCollection_i.cpp
    TDeviceHub_i.cpp
    TDevice_i.cpp
    TDeviceMessageDispatcher_i.cpp
    TDeviceMessageHandler_i.cpp
    TEventEngine_i.cpp
    TEventEngineScheduler_i.cpp
    TParsedShot_i.cpp
    TRefreshIndicator_i.cpp
    TTriggerCallback_i.cpp
)

target_link_libraries(stinetwork 
    pthread
    omnithread
    omniORB4
    omniDynamic4
    omniCodeSets4
    # omnisslTP4   #if OpenSSL is available
    COS4
    COSDynamic4
)

#set_target_properties(stinetwork PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(stinetwork PROPERTIES
    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER api/mylib.h
)

target_include_directories(stinetwork PRIVATE 
    ${WORKING_DIRECTORY}
    ${STI_BASE_DIR}/include
    ${STI_BASE_DIR}/src/device/src
    # ${WORKING_DIRECTORY}/fwd
    # ${WORKING_DIRECTORY}/utils
)

install(TARGETS stinetwork
    DESTINATION ${STI_BASE_DIR}/lib
    #LIBRARY DESTINATION ${STI_BASE_DIR}/lib
    #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# target_link_libraries(testDevice stidevice stinetwork)
