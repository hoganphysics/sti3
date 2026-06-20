cmake_minimum_required(VERSION 3.14...3.26)

if(NOT DEFINED STI3_VERSION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/StiVersion.cmake")
  sti3_load_version("${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endif()

# set the project name
project(STIServer VERSION ${STI3_VERSION} DESCRIPTION "STI Server" LANGUAGES CXX)

include(GNUInstallDirs)

get_filename_component(STI3_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(STI3_LOCAL_BUILD_DIR "${STI3_SOURCE_ROOT}/build-ninja" CACHE PATH
  "Path to the local sti3 build tree used by standalone STIServer builds"
)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()
find_package(Boost 1.70 REQUIRED)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)  # omniORB headers use 'requires' identifier, incompatible with C++20
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
  cmake_policy(SET CMP0091 NEW)
  # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif (MSVC)


function(sti3_find_first_existing_file out_var)
  foreach(_path IN LISTS ARGN)
    if(EXISTS "${_path}")
      set(${out_var} "${_path}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

function(sti3_import_local_build_targets)
  if(NOT IS_ABSOLUTE "${STI3_LOCAL_BUILD_DIR}")
    get_filename_component(_sti3_local_build_dir
      "${STI3_SOURCE_ROOT}/${STI3_LOCAL_BUILD_DIR}" ABSOLUTE)
  else()
    set(_sti3_local_build_dir "${STI3_LOCAL_BUILD_DIR}")
  endif()

  sti3_find_first_existing_file(_stidevice_library
    "${_sti3_local_build_dir}/src/device/src/libstidevice.so"
    "${_sti3_local_build_dir}/lib/libstidevice.so"
  )
  sti3_find_first_existing_file(_stinetwork_library
    "${_sti3_local_build_dir}/src/network/src/libstinetwork.so"
    "${_sti3_local_build_dir}/lib/libstinetwork.so"
  )

  if(NOT (_stidevice_library AND _stinetwork_library))
    message(STATUS "Local sti3 libraries not found in ${_sti3_local_build_dir}")
    return()
  endif()

  set(_sti3_include_dirs "${STI3_SOURCE_ROOT}/include")
  if(EXISTS "${_sti3_local_build_dir}/generated/include")
    list(APPEND _sti3_include_dirs "${_sti3_local_build_dir}/generated/include")
  endif()

  if(NOT TARGET sti3::stidevice)
    add_library(sti3::stidevice SHARED IMPORTED)
    set_target_properties(sti3::stidevice PROPERTIES
      IMPORTED_LOCATION "${_stidevice_library}"
      INTERFACE_INCLUDE_DIRECTORIES "${_sti3_include_dirs}"
    )
  endif()

  if(NOT TARGET sti3::stinetwork)
    add_library(sti3::stinetwork SHARED IMPORTED)
    set_target_properties(sti3::stinetwork PROPERTIES
      IMPORTED_LOCATION "${_stinetwork_library}"
      INTERFACE_INCLUDE_DIRECTORIES "${_sti3_include_dirs}"
      INTERFACE_LINK_LIBRARIES sti3::stidevice
    )
  endif()

  get_filename_component(_stidevice_library_dir "${_stidevice_library}" DIRECTORY)
  get_filename_component(_stinetwork_library_dir "${_stinetwork_library}" DIRECTORY)
  set(_dependency_library_dirs "")

  foreach(_prefix IN ITEMS "$ENV{CONDA_PREFIX}" "$ENV{PREFIX}")
    if(NOT "${_prefix}" STREQUAL "" AND EXISTS "${_prefix}/lib")
      list(APPEND _dependency_library_dirs "${_prefix}/lib")
    endif()
  endforeach()

  if(EXISTS "${_sti3_local_build_dir}/CMakeCache.txt")
    file(STRINGS "${_sti3_local_build_dir}/CMakeCache.txt" _omniorb_lib_line
      REGEX "^OMNIORB_LIB:FILEPATH="
    )
    if(_omniorb_lib_line)
      string(REGEX REPLACE "^[^=]+=" "" _omniorb_lib "${_omniorb_lib_line}")
      get_filename_component(_omniorb_library_dir "${_omniorb_lib}" DIRECTORY)
      list(APPEND _dependency_library_dirs "${_omniorb_library_dir}")
    endif()

    file(STRINGS "${_sti3_local_build_dir}/CMakeCache.txt" _exe_link_flags_line
      REGEX "^CMAKE_EXE_LINKER_FLAGS:STRING="
    )
    if(_exe_link_flags_line)
      string(REGEX REPLACE "^[^=]+=" "" _exe_link_flags "${_exe_link_flags_line}")
      string(REGEX MATCHALL "(^| )-L[^ ]+" _link_dir_flags "${_exe_link_flags}")
      foreach(_link_dir_flag IN LISTS _link_dir_flags)
        string(STRIP "${_link_dir_flag}" _link_dir_flag)
        string(REGEX REPLACE "^-L" "" _link_dir "${_link_dir_flag}")
        if(EXISTS "${_link_dir}")
          list(APPEND _dependency_library_dirs "${_link_dir}")
        endif()
      endforeach()
    endif()
  endif()

  if(_dependency_library_dirs)
    list(REMOVE_DUPLICATES _dependency_library_dirs)
  endif()

  set(STI3_USING_LOCAL_BUILD_TARGETS ON PARENT_SCOPE)
  set(STI3_LOCAL_RUNTIME_LIBRARY_DIRS
    "${_stinetwork_library_dir};${_stidevice_library_dir}" PARENT_SCOPE)
  set(STI3_LOCAL_DEPENDENCY_LIBRARY_DIRS
    "${_dependency_library_dirs}" PARENT_SCOPE)

  message(STATUS "Using local sti3 libraries from ${_sti3_local_build_dir}")
endfunction()

function(sti3_import_installed_targets)
  set(_sti3_prefix_hints "")

  foreach(_prefix IN ITEMS "$ENV{CONDA_PREFIX}" "$ENV{PREFIX}")
    if(NOT "${_prefix}" STREQUAL "")
      list(APPEND _sti3_prefix_hints "${_prefix}")
    endif()
  endforeach()

  foreach(_prefix IN LISTS CMAKE_PREFIX_PATH)
    if(NOT "${_prefix}" STREQUAL "")
      list(APPEND _sti3_prefix_hints "${_prefix}")
    endif()
  endforeach()

  if(_sti3_prefix_hints)
    list(REMOVE_DUPLICATES _sti3_prefix_hints)
  endif()

  find_path(STI3_INSTALLED_INCLUDE_DIR
    NAMES sti/sti.h
    HINTS ${_sti3_prefix_hints}
    PATH_SUFFIXES include
  )
  find_library(STI3_STIDEVICE_LIBRARY
    NAMES stidevice
    HINTS ${_sti3_prefix_hints}
    PATH_SUFFIXES lib
  )
  find_library(STI3_STINETWORK_LIBRARY
    NAMES stinetwork
    HINTS ${_sti3_prefix_hints}
    PATH_SUFFIXES lib
  )

  if(STI3_INSTALLED_INCLUDE_DIR AND STI3_STIDEVICE_LIBRARY AND STI3_STINETWORK_LIBRARY)
    if(NOT TARGET sti3::stidevice)
      add_library(sti3::stidevice SHARED IMPORTED)
      set_target_properties(sti3::stidevice PROPERTIES
        IMPORTED_LOCATION "${STI3_STIDEVICE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${STI3_INSTALLED_INCLUDE_DIR}"
      )
    endif()

    if(NOT TARGET sti3::stinetwork)
      add_library(sti3::stinetwork SHARED IMPORTED)
      set_target_properties(sti3::stinetwork PROPERTIES
        IMPORTED_LOCATION "${STI3_STINETWORK_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${STI3_INSTALLED_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES sti3::stidevice
      )
    endif()

    get_filename_component(_sti3_installed_library_dir "${STI3_STINETWORK_LIBRARY}" DIRECTORY)
    set(STI3_USING_INSTALLED_TARGETS ON PARENT_SCOPE)
    message(STATUS "Using installed sti3 libraries from ${_sti3_installed_library_dir}")
  endif()
endfunction()

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

if(NOT (TARGET sti3::stidevice AND TARGET sti3::stinetwork))
  sti3_import_local_build_targets()
endif()

if(NOT (TARGET sti3::stidevice AND TARGET sti3::stinetwork))
  sti3_import_installed_targets()
endif()

if(NOT (TARGET sti3::stidevice AND TARGET sti3::stinetwork))
  find_package(sti3 CONFIG REQUIRED)
endif()

# # TestNetworkWrapper.h pulls in omniORB headers via generated/deviceNet.h.
# if(NOT TARGET OmniORB::omnithread)
#   find_package(omniORB CONFIG QUIET)
# endif()


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

# if (MSVC)
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#     set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
# endif (MSVC)

set(STI_BASE_DIR ../../..)

#FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )

# link_directories(${STI_BASE_DIR}/lib)

#add_definitions(-D_ITERATOR_DEBUG_LEVEL=0) 

set(TINYXML_SOURCES
  tinyxml/tinyxml2.cpp
)

# add the executable
add_executable(STIServer 
    main.cpp 
    LegacyExperimentXMLBuilder.cpp
    LegacyExperimentXMLReader.cpp
    LegacyParseXMLBuilder.cpp
    LegacyShotRepository.cpp
    LegacySequenceXMLBuilder.cpp
    ServerDevice.cpp
    ${TINYXML_SOURCES}
)
# set_property(TARGET STIServer 
#     PROPERTY POSITION_INDEPENDENT_CODE ON
#     #MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
#     #MSVC_RUNTIME_LIBRARY "MultiThreaded"
#     MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
#     )

# target_include_directories(STIServer PRIVATE 
#     ${WORKING_DIRECTORY}
#     tinyxml
# #    ${Boost_INCLUDE_DIR}
#     ${STI_BASE_DIR}/include
#     ${STI_BASE_DIR}/src/device/src
# #    ${STI_BASE_DIR}/src/network/src
# )

# # temp:
# if(MSVC)
#   target_compile_definitions(STIServer PRIVATE
#     __WIN32__ __NT__ __OSVERSION__=4 __x86__
#     _WIN32_WINNT=0x0400 _WINDOWS
#   )
#   # add_compile_definitions(__WIN32__ __x86__ _WIN32_WINNT=0x0400 __NT__ __OSVERSION__=4 _WINDOWS) 
# endif()


target_include_directories(STIServer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/sti/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/src/
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../network/src/
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../network/src/generated/
)

if(Boost_INCLUDE_DIRS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  foreach(_boost_include_dir IN LISTS Boost_INCLUDE_DIRS)
    get_filename_component(_boost_include_realpath "${_boost_include_dir}" REALPATH)
    set(_boost_include_is_implicit OFF)

    foreach(_implicit_include_dir IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
      if(EXISTS "${_implicit_include_dir}")
        get_filename_component(_implicit_include_realpath
          "${_implicit_include_dir}" REALPATH)
        if(_boost_include_realpath STREQUAL _implicit_include_realpath)
          set(_boost_include_is_implicit ON)
        endif()
      endif()
    endforeach()

    if(NOT _boost_include_is_implicit)
      target_compile_options(STIServer PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-idirafter ${_boost_include_dir}>"
      )
    endif()
  endforeach()
endif()

target_link_libraries(STIServer
  PRIVATE
    sti3::stidevice
    sti3::stinetwork
    Boost::headers
)

# if(TARGET OmniORB::omnithread AND TARGET OmniORB::omniORB4 AND TARGET OmniORB::omniDynamic4)
#   target_link_libraries(STIServer
#     PRIVATE
#       OmniORB::omnithread
#       OmniORB::omniORB4
#       OmniORB::omniDynamic4
#   )
# else()
#   # Fallback for builds without OmniORB CMake targets (e.g., local conda envs).
#   set(_CONDAPREFIX $ENV{PREFIX})
#   if(NOT _CONDAPREFIX)
#     set(_CONDAPREFIX $ENV{CONDA_PREFIX})
#   endif()
#   set(_omni_lib_hints "")
#   if(_CONDAPREFIX)
#     list(APPEND _omni_lib_hints "${_CONDAPREFIX}/lib")
#   endif()
#   find_library(OMNITHREAD_LIB NAMES omnithread omnithread4 HINTS ${_omni_lib_hints})
#   find_library(OMNIORB4_LIB NAMES omniORB4 HINTS ${_omni_lib_hints})
#   find_library(OMNIDYNAMIC4_LIB NAMES omniDynamic4 HINTS ${_omni_lib_hints})
#   if(OMNITHREAD_LIB AND OMNIORB4_LIB AND OMNIDYNAMIC4_LIB)
#     target_link_libraries(STIServer
#       PRIVATE
#         ${OMNITHREAD_LIB}
#         ${OMNIORB4_LIB}
#         ${OMNIDYNAMIC4_LIB}
#     )
#   endif()
# endif()

# RPATH so the executable finds your shared libs in $PREFIX/lib at runtime
# (No-op on Windows)
if(APPLE)
  set_target_properties(STIServer PROPERTIES
    INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
    BUILD_WITH_INSTALL_RPATH OFF
  )
elseif(UNIX)
  set_target_properties(STIServer PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    BUILD_WITH_INSTALL_RPATH OFF
  )
endif()

if(UNIX AND STI3_USING_INSTALLED_TARGETS)
  set_target_properties(STIServer PROPERTIES
    SKIP_BUILD_RPATH ON
  )
endif()

if(UNIX AND STI3_USING_LOCAL_BUILD_TARGETS)
  if(STI3_LOCAL_DEPENDENCY_LIBRARY_DIRS)
    target_link_options(STIServer PRIVATE "-Wl,--no-as-needed")
    target_link_directories(STIServer PRIVATE ${STI3_LOCAL_DEPENDENCY_LIBRARY_DIRS})
    target_link_libraries(STIServer PRIVATE curl)
  endif()

  set_target_properties(STIServer PROPERTIES
    BUILD_RPATH "${STI3_LOCAL_RUNTIME_LIBRARY_DIRS};${STI3_LOCAL_DEPENDENCY_LIBRARY_DIRS}"
  )
endif()

# target_link_directories(STIServer PUBLIC 
#     "C:/Users/Jason/Code/dev/sti3/src/device/src/build/install/x64-Debug/lib"
#     "C:/Users/Jason/Code/dev/sti3/src/network/src/build/install/x64-Debug/lib"
# )

# target_link_libraries(STIServer 
# stidevice${STI_LIB_POSTFIX} 
# stinetwork${STI_LIB_POSTFIX}
# )

# install(TARGETS STIServer
#     #DESTINATION ${STI_BASE_DIR}/lib
#     DESTINATION ../../src/server/bin   #Prepends DESTDIR from command line. Here using ../../ to escape the default /usr/local prefix (hack)
#     #PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )


# Install the exe to <prefix>/bin (Conda puts <env>/bin on PATH)
install(TARGETS STIServer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # exe/.dll launcher on Windows; exe on Linux/macOS
  BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR}  # harmless elsewhere
)

if(MSVC AND STI3_INSTALL_PDBS)
  install(FILES
    $<TARGET_PDB_FILE:STIServer>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL
  )
endif()
