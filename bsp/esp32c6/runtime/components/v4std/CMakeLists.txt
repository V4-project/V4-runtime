# V4-std ESP-IDF Component Links to V4-std library in V4-project/V4-std

# Try multiple possible locations for V4-std 1. Parent-provided V4STD_DIR variable
# (preferred) 2. Docker: /v4-std 3. CI: runtime/_deps/V4-std 4. Local:
# ../../../../../V4-std

set(V4STD_DIR_DOCKER "/v4-std")
set(V4STD_DIR_CI "${CMAKE_CURRENT_LIST_DIR}/../../_deps/V4-std")
set(V4STD_DIR_LOCAL "${CMAKE_CURRENT_LIST_DIR}/../../../../../V4-std")

message(STATUS "V4STD: CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "V4STD: Checking CI path = ${V4STD_DIR_CI}")
message(STATUS "V4STD: Looking for ${V4STD_DIR_CI}/include/v4std/ddt.h")

if(DEFINED V4STD_DIR)
  # Passed from parent CMakeLists.txt
  message(STATUS "Using V4-std from parent: ${V4STD_DIR}")
elseif(EXISTS "${V4STD_DIR_DOCKER}/include/v4std/ddt.h")
  set(V4STD_DIR "${V4STD_DIR_DOCKER}")
  message(STATUS "Found V4-std at ${V4STD_DIR} (Docker location)")
elseif(EXISTS "${V4STD_DIR_CI}/include/v4std/ddt.h")
  set(V4STD_DIR "${V4STD_DIR_CI}")
  message(STATUS "Found V4-std at ${V4STD_DIR} (CI location)")
elseif(EXISTS "${V4STD_DIR_LOCAL}/include/v4std/ddt.h")
  set(V4STD_DIR "${V4STD_DIR_LOCAL}")
  message(STATUS "Found V4-std at ${V4STD_DIR} (local location)")
else()
  message(
    FATAL_ERROR
      "V4-std not found. Please clone V4-std repository to:\n"
      "  - ${V4STD_DIR_DOCKER} (Docker build)\n" "  - ${V4STD_DIR_CI} (CI build)\n"
      "  - ${V4STD_DIR_LOCAL} (local build)")
endif()

# Register component with V4-std sources
idf_component_register(
  SRCS
  "${V4STD_DIR}/src/ddt.cpp"
  "${V4STD_DIR}/src/sys_handlers.cpp"
  "${V4STD_DIR}/src/sys_led.cpp"
  INCLUDE_DIRS
  "${V4STD_DIR}/include"
  REQUIRES)

# Create generated directory before build
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/v4std")

# Generate sys_ids.h from v4sys_ids.def
set(V4SYS_IDS_DEF "${V4STD_DIR}/include/v4std/v4sys_ids.def")
set(V4SYS_IDS_H "${CMAKE_CURRENT_BINARY_DIR}/generated/v4std/sys_ids.h")

add_custom_command(
  OUTPUT "${V4SYS_IDS_H}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/v4std"
  COMMAND ${CMAKE_COMMAND} -DINPUT_FILE=${V4SYS_IDS_DEF} -DOUTPUT_FILE=${V4SYS_IDS_H} -P
          ${V4STD_DIR}/cmake/generate_sys_ids.cmake
  DEPENDS "${V4SYS_IDS_DEF}" "${V4STD_DIR}/cmake/generate_sys_ids.cmake"
  COMMENT "Generating sys_ids.h from v4sys_ids.def"
  VERBATIM)

add_custom_target(generate_sys_ids_esp DEPENDS "${V4SYS_IDS_H}")
add_dependencies(${COMPONENT_LIB} generate_sys_ids_esp)

# Add generated directory to include path
target_include_directories(${COMPONENT_LIB}
                           PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/generated")
