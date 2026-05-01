function(sti3_load_version root_dir)
  set(_sti3_version_file "${root_dir}/sti3_version.json")

  if(NOT EXISTS "${_sti3_version_file}")
    message(FATAL_ERROR "STI3 version file not found: ${_sti3_version_file}")
  endif()

  file(READ "${_sti3_version_file}" _sti3_version_json)
  string(JSON _sti3_version GET "${_sti3_version_json}" version)
  string(JSON _sti3_build_number GET "${_sti3_version_json}" build_number)

  set(STI3_VERSION "${_sti3_version}" PARENT_SCOPE)
  set(STI3_BUILD_NUMBER "${_sti3_build_number}" PARENT_SCOPE)
endfunction()
