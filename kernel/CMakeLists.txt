cmake_minimum_required(VERSION 3.15)

# ------------------------------------------------------------
# V4 Headers (dependency)
# ------------------------------------------------------------
if(DEFINED V4_SRC_DIR)
  # Use local V4 source directory
  set(_V4_SRC_ROOT "${V4_SRC_DIR}")
  if(NOT EXISTS "${_V4_SRC_ROOT}")
    message(FATAL_ERROR "V4_SRC_DIR not found: ${_V4_SRC_ROOT}")
  endif()
  set(_V4_INC_SRC "${_V4_SRC_ROOT}/include")
  message(STATUS "Using V4 headers from: ${_V4_SRC_ROOT}")

elseif(DEFINED V4_INCLUDE_DIR)
  # Use V4 include directory only
  set(_V4_INC_SRC "${V4_INCLUDE_DIR}")
  if(NOT EXISTS "${_V4_INC_SRC}")
    message(FATAL_ERROR "V4_INCLUDE_DIR not found: ${_V4_INC_SRC}")
  endif()
  message(STATUS "Using V4 headers from: ${_V4_INC_SRC}")

elseif(V4_FETCH)
  # Fetch V4 from Git
  include(FetchContent)

  set(V4_GIT_REPO
      "https://github.com/V4-project/V4.git"
      CACHE STRING "V4 Git repository")
  set(V4_GIT_TAG
      "main"
      CACHE STRING "V4 Git tag/branch")

  set(V4_BUILD_TESTS
      OFF
      CACHE BOOL "Build V4 tests" FORCE)

  fetchcontent_declare(
    v4_src
    GIT_REPOSITORY ${V4_GIT_REPO}
    GIT_TAG ${V4_GIT_TAG})
  fetchcontent_makeavailable(v4_src)

  set(_V4_INC_SRC "${v4_src_SOURCE_DIR}/include")
  message(STATUS "Fetched V4 from: ${v4_src_SOURCE_DIR}")

else()
  # Try local sibling directory as default
  get_filename_component(_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(_V4_DEFAULT "${_PARENT_DIR}/V4")

  if(EXISTS "${_V4_DEFAULT}/include")
    set(_V4_INC_SRC "${_V4_DEFAULT}/include")
    message(STATUS "Using V4 headers from sibling directory: ${_V4_DEFAULT}")
  else()
    message(
      FATAL_ERROR
        "V4 dependency not specified and not found in sibling directory.\n"
        "Use one of:\n" "  -DV4_SRC_DIR=/path/to/V4\n"
        "  -DV4_INCLUDE_DIR=/path/to/V4/include\n" "  -DV4_FETCH=ON")
  endif()
endif()

# V4 interface target
add_library(v4headers INTERFACE)
target_include_directories(v4headers INTERFACE "${_V4_INC_SRC}")

# ------------------------------------------------------------
# ------------------------------------------------------------
# Kernel Library
# ------------------------------------------------------------
add_library(v4_kernel STATIC src/vm.c src/vm_exec.c src/scheduler.c src/message.c
                             src/task.c src/platform_stub.c)

target_include_directories(v4_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(v4_kernel PUBLIC v4headers)

target_compile_options(v4_kernel PRIVATE -Wall -Wextra -Werror $<$<CONFIG:Debug>:-g -O0>
                                         $<$<CONFIG:Release>:-O2>)
