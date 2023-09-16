#!/bin/bash -e

cd $(dirname "${BASH_SOURCE[0]}")

build_dir=/tmp/aop-build

cmake -B "$build_dir" .
cmake --build "$build_dir"
ctest -V --test-dir "$build_dir"
