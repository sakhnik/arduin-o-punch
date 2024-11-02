#!/bin/bash -e

cd $(dirname "${BASH_SOURCE[0]}")

em++ \
    -lembind \
    aop.cpp ../lib/aop/*.cpp \
    -o aop.js \
    -DBUILD_WA=1

    #-s SINGLE_FILE=1
