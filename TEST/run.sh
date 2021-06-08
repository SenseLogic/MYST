#!/bin/sh
set -x
../BUILD/xenius --read-e57-cloud IN/triangle.e57 --write-xyz-cloud OUT/triangle.xyz
../BUILD/xenius --read-e57-cloud IN/triangle_2.e57 --write-xyz-cloud OUT/triangle_2.xyz
../BUILD/xenius --read-e57-cloud IN/triangle_3.e57 --write-xyz-cloud OUT/triangle_3.xyz
../BUILD/xenius --read-e57-cloud IN/cube.e57 --write-xyz-cloud OUT/cube.xyz
../BUILD/xenius --read-e57-cloud IN/cube_2.e57 --write-xyz-cloud OUT/cube_2.xyz
../BUILD/xenius --read-e57-cloud IN/cube_3.e57 --write-xyz-cloud OUT/cube_3.xyz
../BUILD/xenius --read-e57-cloud IN/teapot.e57 --write-xyz-cloud OUT/teapot.xyz
../BUILD/xenius --read-e57-cloud IN/bunny.e57 --write-xyz-cloud OUT/bunny.xyz
../BUILD/xenius --read-e57-cloud IN/bunny.e57 --write-pts-cloud OUT/bunny.pts XYZIRGB
../BUILD/xenius --position-rotation 0 0 -45 --read-e57-cloud IN/bunny.e57 --write-xyz-cloud OUT/rotated_bunny.xyz
