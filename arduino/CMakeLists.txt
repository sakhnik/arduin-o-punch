cmake_minimum_required(VERSION 3.10)

project(AOP VERSION 2.1.0)

set(PROFILE "c3" CACHE STRING "Specify the Arduino CLI profile (c3, pro, pro16)")
if(NOT DEFINED PROFILE)
    message(FATAL_ERROR "Profile name not specified. Use -DPROFILE=<profile_name> to set it.")
endif()

set(PORT "/dev/ttyUSB0" CACHE STRING "Specify serial port to upload the firmware to")
set(ARCH "avr")
if(${PROFILE} STREQUAL "c3")
    set(PORT "/dev/ttyACM0")
    set(ARCH "esp32")
endif()

message(STATUS "Using profile: ${PROFILE}")
message(STATUS "Using port: ${PORT}")

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")
set(TARGET_SUFFIX "bin")
if(${PROFILE} STREQUAL "pro" OR ${PROFILE} STREQUAL "pro16")
    set(TARGET_SUFFIX "hex")
endif()
set(TARGET_FILE "${CMAKE_BINARY_DIR}/arduino.ino.${TARGET_SUFFIX}")

set(ADDITIONAL_DEFINES "-DGIT_REVISION=\"\\\"${GIT_REVISION}\\\"\" -DPROJECT_VERSION=\"\\\"${PROJECT_VERSION}\\\"\"")
set(EXTRA_FLAGS --build-property "compiler.cpp.extra_flags=${ADDITIONAL_DEFINES}")
set(COMPILE_COMMAND arduino-cli compile -m ${PROFILE} --build-path ${CMAKE_BINARY_DIR} ${EXTRA_FLAGS})
add_custom_command(
    OUTPUT ${TARGET_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling with profile ${PROFILE}"
    COMMAND ${COMPILE_COMMAND}
    COMMAND ${COMPILE_COMMAND} --only-compilation-database
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET_FILE} "${CMAKE_BINARY_DIR}/aop-${PROFILE}.${ARCH}.${TARGET_SUFFIX}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${SOURCES}
    COMMENT "Checking for source file changes and compiling if necessary..."
)

add_custom_target(compile ALL DEPENDS ${TARGET_FILE})

add_custom_target(upload
    COMMAND arduino-cli upload -p ${PORT} -m ${PROFILE} --input-dir ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Uploading with profile ${PROFILE} to ${PORT}"
)

add_dependencies(upload compile)
