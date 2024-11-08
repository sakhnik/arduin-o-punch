#!/bin/bash -e

astyle --project '*.cpp,*.h,*.ino'
./lib/aop/test.sh
#pio run -e esp32c3 -j3
#pio run -e pro -j3
pio run -j3
