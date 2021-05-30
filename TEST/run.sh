#!/bin/sh
set -x
../BUILD/xenius IN/triangle.e57 OUT/triangle.xyz
../BUILD/xenius IN/triangle_2.e57 OUT/triangle_2.xyz
../BUILD/xenius IN/triangle_3.e57 OUT/triangle_3.xyz
../BUILD/xenius IN/cube.e57 OUT/cube.xyz
../BUILD/xenius IN/cube_2.e57 OUT/cube_2.xyz
../BUILD/xenius IN/cube_3.e57 OUT/cube_3.xyz
../BUILD/xenius IN/teapot.e57 OUT/teapot.xyz
../BUILD/xenius IN/bunny.e57 OUT/bunny.xyz
