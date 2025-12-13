cmake_minimum_required(VERSION 3.20)

# If you're already setting this in the top-level, you can omit it here.
# project(STI3Tests LANGUAGES CXX)

find_package(Catch2 3 CONFIG REQUIRED)

if(NOT (TARGET sti3::stidevice AND TARGET sti3::stinetwork))
  find_package(sti3 CONFIG REQUIRED)
endif()

# Catch2 provides a CMake helper to auto-register tests with ctest
include(Catch)

# ---- Device tests ----
add_executable(sti3_test_device
  device/utils_tests.cpp
  device/deviceid_tests.cpp
  device/mixedvalue_tests.cpp
  device/configuration_tests.cpp
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
  device/devicemessagelistener_tests.cpp
  device/localdevicemessagehandler_tests.cpp
  device/localdevicemessagedispatcher_tests.cpp
  device/devicemessagereceiver_tests.cpp
  # add more device test .cpp files here
)

target_link_libraries(sti3_test_device
  PRIVATE
    Catch2::Catch2WithMain
    sti3::stidevice
    # link your device lib target here:
    # sti3_device
)

# target_compile_features(sti3_test_device PRIVATE cxx_std_17)

catch_discover_tests(sti3_test_device)

# # ---- Network tests ----
# add_executable(sti3_test_network
#   network/test_network_example.cpp
#   # add more network test .cpp files here
# )

# target_link_libraries(sti3_test_network
#   PRIVATE
#     Catch2::Catch2WithMain
#     # link your network lib target here:
#     # sti3_network
# )

# target_compile_features(sti3_test_network PRIVATE cxx_std_17)

# catch_discover_tests(sti3_test_network)
