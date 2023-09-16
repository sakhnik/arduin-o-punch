cmake_minimum_required(VERSION 3.16)

project(AopTest)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DBUILD_TEST)

add_executable(test-aop
    IMifare.h
    PunchCard.cpp
    PunchCard.h
    PunchCardTest.cpp
    Punch.h
    PunchTest.cpp
    test.cpp
    )

enable_testing()

add_test(aop test-aop)
