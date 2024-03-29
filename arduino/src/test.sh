#!/bin/bash -e

cd $(dirname "${BASH_SOURCE[0]}")

build_dir=/tmp/aop-build

cmake -B "$build_dir" -DCMAKE_BUILD_TYPE=Debug .
cmake --build "$build_dir"
if [[ "$1" != "--skip-tests" ]]; then
    ctest -V --test-dir "$build_dir"
fi
