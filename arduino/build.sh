#!/bin/bash -e

astyle --project '*.cpp,*.h,*.ino'
./src/test.sh
arduino-cli compile -m nano
