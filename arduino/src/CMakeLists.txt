cmake_minimum_required(VERSION 3.16)

project(AopTest)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DBUILD_TEST)
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")

add_executable(test-aop
    IKeyReceiver.h
    IMifare.h
    Punch.h
    PunchCard.cpp
    PunchCard.h
    PunchCardTest.cpp
    PunchTest.cpp
    Recorder.cpp
    Recorder.h
    RecorderTest.cpp
    test.cpp
    )

enable_testing()

add_test(aop test-aop)
