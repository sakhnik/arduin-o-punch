#!/bin/bash -e

cd $(dirname "${BASH_SOURCE[0]}")

em++ \
    -lembind \
    aop.cpp ../src/*.cpp \
    -o aop.js \
    -DBUILD_WA=1

    #-s SINGLE_FILE=1
