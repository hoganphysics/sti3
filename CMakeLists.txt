cmake_minimum_required(VERSION 3.20)
project(sti3)

#set(STI_BASE_DIR ${WORKING_DIRECTORY})
set(STI_BASE_DIR ${sti3_SOURCE_DIR})
#set(STI_BASE_DIR "C:/Users/Jason/Code/dev/sti3") #${sti3_SOURCE_DIR}
message("STI_BASE_DIR: ${STI_BASE_DIR}")

find_package(Python COMPONENTS Interpreter Development REQUIRED)


# Find pybind11 cmake directory
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c
        "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE _tmp_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

add_subdirectory(src/device/src)
add_subdirectory(src/network/src)
add_subdirectory(src/stipy/stidevicepy/src)
add_subdirectory(src/stipy/src)

#install(TARGETS stipybase stidevicepy stipy
#    LIBRARY DESTINATION "src/stipy/bin"
#)

