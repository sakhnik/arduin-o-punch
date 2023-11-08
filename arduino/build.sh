#!/bin/bash -e

./src/test.sh
arduino-cli compile -m nano
