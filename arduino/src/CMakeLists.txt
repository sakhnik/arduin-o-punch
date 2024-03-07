cmake_minimum_required(VERSION 3.16)

project(AopTest)

include(FetchContent)
FetchContent_Declare(
    DocTest
    GIT_REPOSITORY "https://github.com/doctest/doctest"
    GIT_TAG "v2.4.11"
)

FetchContent_MakeAvailable(DocTest)
include_directories(${doctest_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DBUILD_TEST)
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")

add_library(aop
    IMifare.cpp
    IMifare.h
    Punch.h
    PunchCard.cpp
    PunchCard.h
    Recorder.cpp
    Recorder.h
)

add_executable(test-aop
    PunchCardTest.cpp
    PunchTest.cpp
    RecorderTest.cpp
    test.cpp
    )
target_link_libraries(test-aop aop)

enable_testing()

add_test(aop test-aop)
