cmake_minimum_required(VERSION 3.25.0)

set(EXTRA_ZEPHYR_MODULES ${EXTRA_ZEPHYR_MODULES} ${CMAKE_CURRENT_SOURCE_DIR}/module)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(esp-hosted-4-zephyr)

add_subdirectory(src)