# SPDX-License-Identifier: MIT OR Apache-2.0

# Resolve relative overrides against the IDF project, never the caller's cwd.
get_filename_component(V4_RUNTIME_IDF_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

function(v4_resolve_dependency output environment repository docker_path marker)
  if(DEFINED ${output} AND NOT "${${output}}" STREQUAL "")
    set(candidates "${${output}}")
  elseif(DEFINED ENV{${environment}} AND NOT "$ENV{${environment}}" STREQUAL "")
    set(candidates "$ENV{${environment}}")
  else()
    set(candidates "${V4_RUNTIME_IDF_DIR}/_deps/${repository}" "${docker_path}"
                   "${V4_RUNTIME_IDF_DIR}/../../../../${repository}")
  endif()
  foreach(candidate IN LISTS candidates)
    get_filename_component(candidate "${candidate}" ABSOLUTE BASE_DIR
                           "${V4_RUNTIME_IDF_DIR}")
    if(EXISTS "${candidate}/${marker}")
      message(STATUS "Found ${repository} at ${candidate}")
      set(${output}
          "${candidate}"
          PARENT_SCOPE)
      return()
    endif()
  endforeach()
  message(FATAL_ERROR "${repository} not found. Checked: ${candidates}. "
                      "Set -D${output}=/path/to/${repository} or ${environment}.")
endfunction()
