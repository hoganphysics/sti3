cmake_minimum_required(VERSION 3.10)

project(fileTransfer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(sti3 CONFIG REQUIRED)

add_executable(fileTransfer main.cpp FileTransferDevice.cpp)

target_link_libraries(fileTransfer
  PRIVATE
    sti3::stidevice
    sti3::stinetwork
)
