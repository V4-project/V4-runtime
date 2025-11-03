cmake_minimum_required(VERSION 3.15)

# ------------------------------------------------------------
# V4 Headers (dependency)
# ------------------------------------------------------------
if(DEFINED V4_SRC_DIR)
  # Use local V4 source directory (full build with VM library)
  set(_V4_SRC_ROOT "${V4_SRC_DIR}")
  if(NOT EXISTS "${_V4_SRC_ROOT}")
    message(FATAL_ERROR "V4_SRC_DIR not found: ${_V4_SRC_ROOT}")
  endif()
  # Disable V4's own tests (we only want V4-rtos tests)
  set(_V4_BUILD_TESTS_SAVED ${V4_BUILD_TESTS})
  set(V4_BUILD_TESTS
      OFF
      CACHE BOOL "Build V4 tests" FORCE)
  add_subdirectory("${_V4_SRC_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/v4_build")
  set(V4_BUILD_TESTS
      ${_V4_BUILD_TESTS_SAVED}
      CACHE BOOL "Build V4-rtos tests" FORCE)
  set(_V4_INC_SRC "${_V4_SRC_ROOT}/include")
  set(_V4_HAS_VM TRUE)
  message(STATUS "Using V4 source from: ${_V4_SRC_ROOT}")

elseif(DEFINED V4_INCLUDE_DIR)
  # Use V4 include directory only (headers-only mode)
  set(_V4_INC_SRC "${V4_INCLUDE_DIR}")
  if(NOT EXISTS "${_V4_INC_SRC}")
    message(FATAL_ERROR "V4_INCLUDE_DIR not found: ${_V4_INC_SRC}")
  endif()
  set(_V4_HAS_VM FALSE)
  message(STATUS "Using V4 headers from: ${_V4_INC_SRC} (headers only)")

elseif(V4_FETCH)
  # Fetch V4 from Git (full build with VM library)
  include(FetchContent)

  set(V4_GIT_REPO
      "https://github.com/V4-project/V4.git"
      CACHE STRING "V4 Git repository")
  set(V4_GIT_TAG
      "main"
      CACHE STRING "V4 Git tag/branch")

  # Disable V4's own tests (we only want V4-rtos tests)
  set(_V4_BUILD_TESTS_SAVED ${V4_BUILD_TESTS})
  set(V4_BUILD_TESTS
      OFF
      CACHE BOOL "Build V4 tests" FORCE)

  fetchcontent_declare(
    v4_src
    GIT_REPOSITORY ${V4_GIT_REPO}
    GIT_TAG ${V4_GIT_TAG})
  fetchcontent_makeavailable(v4_src)

  set(V4_BUILD_TESTS
      ${_V4_BUILD_TESTS_SAVED}
      CACHE BOOL "Build V4-rtos tests" FORCE)

  set(_V4_INC_SRC "${v4_src_SOURCE_DIR}/include")
  set(_V4_HAS_VM TRUE)
  message(STATUS "Fetched V4 from: ${v4_src_SOURCE_DIR}")

else()
  # Try local sibling directory as default
  get_filename_component(_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(_V4_DEFAULT "${_PARENT_DIR}/V4")

  if(EXISTS "${_V4_DEFAULT}/include")
    # Disable V4's own tests (we only want V4-rtos tests)
    set(_V4_BUILD_TESTS_SAVED ${V4_BUILD_TESTS})
    set(V4_BUILD_TESTS
        OFF
        CACHE BOOL "Build V4 tests" FORCE)
    add_subdirectory("${_V4_DEFAULT}" "${CMAKE_CURRENT_BINARY_DIR}/v4_build")
    set(V4_BUILD_TESTS
        ${_V4_BUILD_TESTS_SAVED}
        CACHE BOOL "Build V4-rtos tests" FORCE)
    set(_V4_INC_SRC "${_V4_DEFAULT}/include")
    set(_V4_HAS_VM TRUE)
    message(STATUS "Using V4 source from sibling directory: ${_V4_DEFAULT}")
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
# Kernel Library
# ------------------------------------------------------------
if(_V4_HAS_VM)
  # V4-RTOS kernel: thin C++ wrapper around V4 VM V4 VM already provides scheduler, tasks,
  # and message queue
  add_library(v4_kernel STATIC src/vm_wrapper.cpp src/platform_stub.cpp)
  target_link_libraries(v4_kernel PUBLIC v4headers v4vm mock_hal)
  message(STATUS "Building V4-RTOS kernel (C++, uses V4 VM scheduler/tasks/messages)")
else()
  message(FATAL_ERROR "V4-RTOS requires V4 VM library (_V4_HAS_VM must be TRUE)")
endif()

target_include_directories(v4_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(v4_kernel PRIVATE -Wall -Wextra -Werror $<$<CONFIG:Debug>:-g -O0>
                                         $<$<CONFIG:Release>:-O2>)

# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------
if(V4_BUILD_TESTS AND _V4_HAS_VM)
  enable_testing()

  # VM wrapper tests
  add_executable(test_vm_wrapper tests/test_vm_wrapper.cpp)
  target_link_libraries(test_vm_wrapper PRIVATE v4_kernel v4vm mock_hal)
  add_test(NAME test_vm_wrapper COMMAND test_vm_wrapper)
endif()
