# V4 RTOS Runtime for ESP32-C6
#
# This is the runtime that gets flashed to the device once.
# It initializes the V4 VM, scheduler, and waits for bytecode
# to be loaded via V4-link protocol.
#
# SPDX-License-Identifier: MIT OR Apache-2.0

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(v4-runtime)
