#!/bin/bash -e

em++ aop.cpp ../src/*.cpp -o aop.js -DBUILD_WA=1 -s WASM=1 -s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']"
