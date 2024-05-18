#!/bin/bash -e

astyle --project '*.cpp,*.h,*.ino'
./src/test.sh
cmake -B /tmp/aop-pro -DPROFILE=pro
cmake --build /tmp/aop-pro
