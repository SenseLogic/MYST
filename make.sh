#!/bin/sh
set -x
rm -r BUILD
mkdir BUILD
cd BUILD
cmake ../LINUX
cmake --build .
