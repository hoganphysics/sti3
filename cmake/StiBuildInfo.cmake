set(_STI3_BUILD_INFO_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(sti3_configure_build_info)
  set(_sti3_options)
  set(_sti3_one_value_args ROOT_DIR GENERATED_INCLUDE_DIR_VAR)
  set(_sti3_multi_value_args)
  cmake_parse_arguments(
    STI3_BUILD_INFO
    "${_sti3_options}"
    "${_sti3_one_value_args}"
    "${_sti3_multi_value_args}"
    ${ARGN}
  )

  if(STI3_BUILD_INFO_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown arguments to sti3_configure_build_info: ${STI3_BUILD_INFO_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT STI3_BUILD_INFO_ROOT_DIR)
    message(FATAL_ERROR "sti3_configure_build_info requires ROOT_DIR")
  endif()

  if(NOT STI3_BUILD_INFO_GENERATED_INCLUDE_DIR_VAR)
    message(FATAL_ERROR "sti3_configure_build_info requires GENERATED_INCLUDE_DIR_VAR")
  endif()

  get_filename_component(_sti3_root_dir "${STI3_BUILD_INFO_ROOT_DIR}" ABSOLUTE)

  if(NOT DEFINED STI3_VERSION OR NOT DEFINED STI3_BUILD_NUMBER)
    include("${_STI3_BUILD_INFO_MODULE_DIR}/StiVersion.cmake")
    sti3_load_version("${_sti3_root_dir}")
  endif()

  if(DEFINED ENV{PKG_VERSION} AND NOT "$ENV{PKG_VERSION}" STREQUAL "${STI3_VERSION}")
    message(FATAL_ERROR "Conda PKG_VERSION=$ENV{PKG_VERSION} does not match sti3_version.json version=${STI3_VERSION}")
  endif()

  if(DEFINED ENV{PKG_BUILDNUM} AND NOT "$ENV{PKG_BUILDNUM}" STREQUAL "${STI3_BUILD_NUMBER}")
    message(FATAL_ERROR "Conda PKG_BUILDNUM=$ENV{PKG_BUILDNUM} does not match sti3_version.json build_number=${STI3_BUILD_NUMBER}")
  endif()

  set(STI3_PACKAGE_BUILD_NUMBER "${STI3_BUILD_NUMBER}")
  set(STI3_PACKAGE_BUILD_NUMBER "${STI3_BUILD_NUMBER}" CACHE STRING "STI3 package build number" FORCE)

  set(STI3_BUILD_STRING "" CACHE STRING "STI3 package build string")
  if(DEFINED ENV{PKG_BUILD_STRING} AND "${STI3_BUILD_STRING}" STREQUAL "")
    set(STI3_BUILD_STRING "$ENV{PKG_BUILD_STRING}")
  endif()
  if("${STI3_BUILD_STRING}" STREQUAL "placeholder")
    set(STI3_BUILD_STRING "")
  endif()

  set(STI3_GIT_COMMIT "" CACHE STRING "STI3 git commit for build metadata")
  if("${STI3_GIT_COMMIT}" STREQUAL "" AND DEFINED ENV{STI3_GIT_COMMIT})
    set(STI3_GIT_COMMIT "$ENV{STI3_GIT_COMMIT}")
  endif()

  set(STI3_GIT_DIRTY "" CACHE STRING "STI3 git dirty flag for build metadata")
  if("${STI3_GIT_DIRTY}" STREQUAL "" AND DEFINED ENV{STI3_GIT_DIRTY})
    set(STI3_GIT_DIRTY "$ENV{STI3_GIT_DIRTY}")
  endif()

  if(EXISTS "${_sti3_root_dir}/.git")
    find_package(Git QUIET)
    if(Git_FOUND)
      if("${STI3_GIT_COMMIT}" STREQUAL "")
        execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-parse --short=12 HEAD
          WORKING_DIRECTORY "${_sti3_root_dir}"
          OUTPUT_VARIABLE STI3_GIT_COMMIT
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_QUIET
        )
      endif()
      if("${STI3_GIT_DIRTY}" STREQUAL "")
        execute_process(
          COMMAND "${GIT_EXECUTABLE}" update-index -q --refresh
          WORKING_DIRECTORY "${_sti3_root_dir}"
          ERROR_QUIET
        )
        execute_process(
          COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=no
          WORKING_DIRECTORY "${_sti3_root_dir}"
          OUTPUT_VARIABLE _sti3_git_status
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_QUIET
        )
        if("${_sti3_git_status}" STREQUAL "")
          set(STI3_GIT_DIRTY "0")
        else()
          set(STI3_GIT_DIRTY "1")
        endif()
      endif()

      set(_sti3_git_configure_deps)
      foreach(_sti3_git_path_name IN ITEMS HEAD index packed-refs)
        execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${_sti3_git_path_name}"
          WORKING_DIRECTORY "${_sti3_root_dir}"
          OUTPUT_VARIABLE _sti3_git_path
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_QUIET
        )
        if(NOT "${_sti3_git_path}" STREQUAL "")
          if(NOT IS_ABSOLUTE "${_sti3_git_path}")
            set(_sti3_git_path "${_sti3_root_dir}/${_sti3_git_path}")
          endif()
          if(EXISTS "${_sti3_git_path}")
            list(APPEND _sti3_git_configure_deps "${_sti3_git_path}")
          endif()
        endif()
      endforeach()

      execute_process(
        COMMAND "${GIT_EXECUTABLE}" symbolic-ref -q HEAD
        WORKING_DIRECTORY "${_sti3_root_dir}"
        OUTPUT_VARIABLE _sti3_git_head_ref
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if(NOT "${_sti3_git_head_ref}" STREQUAL "")
        execute_process(
          COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${_sti3_git_head_ref}"
          WORKING_DIRECTORY "${_sti3_root_dir}"
          OUTPUT_VARIABLE _sti3_git_head_ref_path
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_QUIET
        )
        if(NOT "${_sti3_git_head_ref_path}" STREQUAL "")
          if(NOT IS_ABSOLUTE "${_sti3_git_head_ref_path}")
            set(_sti3_git_head_ref_path "${_sti3_root_dir}/${_sti3_git_head_ref_path}")
          endif()
          if(EXISTS "${_sti3_git_head_ref_path}")
            list(APPEND _sti3_git_configure_deps "${_sti3_git_head_ref_path}")
          endif()
        endif()
      endif()

      if(_sti3_git_configure_deps)
        list(REMOVE_DUPLICATES _sti3_git_configure_deps)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_sti3_git_configure_deps})
      endif()
    endif()
  endif()

  if("${STI3_GIT_COMMIT}" STREQUAL "")
    set(STI3_GIT_COMMIT "unknown")
  endif()

  string(TOLOWER "${STI3_GIT_DIRTY}" _sti3_git_dirty_lower)
  if(_sti3_git_dirty_lower STREQUAL "1" OR
     _sti3_git_dirty_lower STREQUAL "true" OR
     _sti3_git_dirty_lower STREQUAL "yes" OR
     _sti3_git_dirty_lower STREQUAL "dirty")
    set(STI3_GIT_DIRTY_INT 1)
  else()
    set(STI3_GIT_DIRTY_INT 0)
  endif()

  if(NOT DEFINED sti3_VERSION_MAJOR OR
     NOT DEFINED sti3_VERSION_MINOR OR
     NOT DEFINED sti3_VERSION_PATCH)
    if("${STI3_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
      set(sti3_VERSION_MAJOR "${CMAKE_MATCH_1}")
      set(sti3_VERSION_MINOR "${CMAKE_MATCH_2}")
      set(sti3_VERSION_PATCH "${CMAKE_MATCH_3}")
    else()
      message(FATAL_ERROR "STI3_VERSION=${STI3_VERSION} does not contain major.minor.patch components")
    endif()
  endif()

  set(STI3_CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
  set(_sti3_generated_include_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
  file(MAKE_DIRECTORY "${_sti3_generated_include_dir}/sti/device")
  configure_file(
    "${_sti3_root_dir}/cmake/VersionBuildInfo.h.in"
    "${_sti3_generated_include_dir}/sti/device/VersionBuildInfo.h"
    @ONLY
  )

  set(${STI3_BUILD_INFO_GENERATED_INCLUDE_DIR_VAR} "${_sti3_generated_include_dir}" PARENT_SCOPE)
  set(STI3_PACKAGE_BUILD_NUMBER "${STI3_PACKAGE_BUILD_NUMBER}" PARENT_SCOPE)
  set(STI3_BUILD_STRING "${STI3_BUILD_STRING}" PARENT_SCOPE)
  set(STI3_GIT_COMMIT "${STI3_GIT_COMMIT}" PARENT_SCOPE)
  set(STI3_GIT_DIRTY "${STI3_GIT_DIRTY}" PARENT_SCOPE)
  set(STI3_GIT_DIRTY_INT "${STI3_GIT_DIRTY_INT}" PARENT_SCOPE)
  set(STI3_CMAKE_BUILD_TYPE "${STI3_CMAKE_BUILD_TYPE}" PARENT_SCOPE)
endfunction()
