cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

# If you're already setting this in the top-level, you can omit it here.
# project(STI3Tests LANGUAGES CXX)
project(sti3_test_device VERSION ${STI3_VERSION} DESCRIPTION "sti3_test_device" LANGUAGES CXX)
include(GNUInstallDirs)

find_package(Catch2 3 CONFIG REQUIRED)
find_package(Boost 1.70 REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

if(NOT (TARGET sti3::stidevice AND TARGET sti3::stinetwork))
  find_package(sti3 CONFIG REQUIRED)
endif()


# ---- Device tests ----
add_executable(sti3_test_device
  device/utils_tests.cpp
  device/deviceid_tests.cpp
  device/mixedvalue_tests.cpp
  device/configuration_tests.cpp
  device/configfile_tests.cpp
  device/dependencytree_tests.cpp
  device/cachedvalue_tests.cpp
  device/synchronizedmap_tests.cpp
  device/metadata_tests.cpp
  device/timestamp_tests.cpp
  device/vectormap_tests.cpp
  device/fileid_tests.cpp
  device/localcollection_tests.cpp
  device/eventqueue_tests.cpp
  device/evaluationbarrier_tests.cpp
  device/localattribute_tests.cpp
  device/localattributemanager_tests.cpp
  device/automonitor_tests.cpp
  device/localmonitor_tests.cpp
  device/localmonitormanager_tests.cpp
  device/devicemessagelistener_tests.cpp
  device/devicemessagerelayer_tests.cpp
  device/devicemessage_tests.cpp
  device/localdevicemessagehandler_tests.cpp
  device/localdevicemessagedispatcher_tests.cpp
  device/devicemessagereceiver_tests.cpp
  device/messagegrouper_tests.cpp
  device/channel_tests.cpp
  device/channelmanager_tests.cpp
  device/localchannel_tests.cpp
  device/localchannelmanager_tests.cpp
  device/localdevice_metadata_tests.cpp
  device/localdevice_partner_tests.cpp
  device/localdevice_write_tests.cpp
  device/localdevicehub_tests.cpp
  device/localfileholder_tests.cpp
  device/localbinarydatastream_tests.cpp
  device/localfileserver_tests.cpp
  device/persistence_import_tests.cpp
  device/localprofilemanager_tests.cpp
  device/binarydata_tests.cpp
  device/image_tests.cpp
  device/measurement_file_tests.cpp
  device/eventengine_value_type_tests.cpp
  device/virtualfileholder_tests.cpp
  device/virtualfileserver_tests.cpp
  device/task_tests.cpp
  device/taskscheduler_tests.cpp
  device/localtaskmanager_tests.cpp
  device/intervaltask_tests.cpp
  device/appointmenttask_tests.cpp
  device/eventengine_missing_partner_tests.cpp
  device/eventengine_stop_engine_tests.cpp
  device/eventengine_sequence_mode_tests.cpp
  device/shotid_tests.cpp
  device/shotresult_tests.cpp
  device/distributer_tests.cpp
  device/locallogmanager_tests.cpp
  device/logger_tests.cpp
  device/logid_tests.cpp
  device/logrecordfile_tests.cpp
  device/versionmanager_tests.cpp
  device/postprocesstarget_tests.cpp
  device/postprocessingmanager_tests.cpp
  device/eventengine_postprocessing_tests.cpp
  device/stidevicepy_parseevents_tests.cpp
  device/stidevicepy_parseevents_tests_support.cpp
  ${CMAKE_SOURCE_DIR}/src/server/src/LegacyExperimentXMLBuilder.cpp
  ${CMAKE_SOURCE_DIR}/src/server/src/tinyxml/tinyxml2.cpp
  # add more device test .cpp files here
)


# target_include_directories(sti3_test_device PRIVATE ${CMAKE_SOURCE_DIR}/src/device/src)

target_include_directories(sti3_test_device PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/device/src>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/stipy/stidevicepy/src>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/server/src>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/server/src/tinyxml>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/sti/extern>
)



target_link_libraries(sti3_test_device
  PRIVATE
    Catch2::Catch2WithMain
    sti3::stidevice
    Boost::headers
    pybind11::embed
    # link your device lib target here:
    # sti3_device
)

# set_target_properties(sti3_test_device PROPERTIES
#   BUILD_RPATH "$<TARGET_FILE_DIR:sti3::stidevice>"
# )

set_target_properties(sti3_test_device PROPERTIES
  BUILD_RPATH "\$ORIGIN/../src/device/src"
  BUILD_RPATH_USE_ORIGIN TRUE
)



# Catch2 provides a CMake helper to auto-register tests with ctest
include(Catch)

set(_sti3_test_device_runtime_dirs)
if(TARGET stidevice)
  list(APPEND _sti3_test_device_runtime_dirs "$<TARGET_FILE_DIR:stidevice>")
elseif(TARGET sti3::stidevice)
  list(APPEND _sti3_test_device_runtime_dirs "$<TARGET_FILE_DIR:sti3::stidevice>")
endif()

catch_discover_tests(sti3_test_device
  DISCOVERY_MODE PRE_TEST
  DL_PATHS ${_sti3_test_device_runtime_dirs}
)



# target_compile_features(sti3_test_device PRIVATE cxx_std_17)

# catch_discover_tests(sti3_test_device)

# ---- Network tests ----
add_executable(sti3_test_network
  network/convert/basic_convert_tests.cpp
  network/convert/enum_convert_tests.cpp
  network/convert/record_convert_tests.cpp
  network/convert/postprocess_convert_tests.cpp
)

set_target_properties(sti3_test_network PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED TRUE
  BUILD_RPATH "\$ORIGIN/../src/network/src;\$ORIGIN/../src/device/src"
  BUILD_RPATH_USE_ORIGIN TRUE
)

target_include_directories(sti3_test_network PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/network/src>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/network/src/convert>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/network/src/generated>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/device/src>
  $<BUILD_INTERFACE:${OMNIORB_INCLUDE_DIR}>
)

if(MSVC)
  target_compile_definitions(sti3_test_network PRIVATE
    __WIN32__
    __x86__
    _WIN32_WINNT=0x0400
    __NT__
    __OSVERSION__=4
    _WINDOWS
    _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
  )
endif()

target_link_libraries(sti3_test_network
  PRIVATE
    Catch2::Catch2WithMain
    sti3::stinetwork
    sti3::stidevice
    Boost::headers
)

if(TARGET OmniORB::omniORB4)
  target_link_libraries(sti3_test_network PRIVATE OmniORB::omniORB4)
endif()

foreach(_sti3_omni_lib IN ITEMS OMNITHREAD_LIB OMNIORB_LIB OMNIDYNAMIC_LIB)
  if(${_sti3_omni_lib})
    target_link_libraries(sti3_test_network PRIVATE ${${_sti3_omni_lib}})
  endif()
endforeach()

catch_discover_tests(sti3_test_network
  DISCOVERY_MODE PRE_TEST
  DL_PATHS ${_sti3_test_device_runtime_dirs} $<TARGET_FILE_DIR:sti3::stinetwork>
)
