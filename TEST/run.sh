#!/bin/sh
set -x
../BUILD/enea --verbose --read-e57-cloud IN/triangle.e57 --write-xyz-cloud OUT/triangle.xyz XYZ
../BUILD/enea --verbose --read-e57-cloud IN/triangle.e57 --write-pcf-cloud OUT/triangle_8.pcf XYZIRGB 8 0.001
../BUILD/enea --verbose --read-e57-cloud IN/triangle.e57 --write-pcf-cloud OUT/triangle_32.pcf XYZIRGB 32 0
../BUILD/enea --verbose --read-e57-cloud IN/triangle_2.e57 --write-xyz-cloud OUT/triangle_2.xyz XYZ
../BUILD/enea --verbose --read-e57-cloud IN/triangle_2.e57 --write-pcf-cloud OUT/triangle_2_8.pcf XYZIRGB 8 0.001
../BUILD/enea --verbose --read-e57-cloud IN/triangle_2.e57 --write-pcf-cloud OUT/triangle_2_32.pcf XYZIRGB 32 0
../BUILD/enea --verbose --read-e57-cloud IN/triangle_3.e57 --write-xyz-cloud OUT/triangle_3.xyz XYZ
../BUILD/enea --verbose --read-e57-cloud IN/triangle_3.e57 --write-pcf-cloud OUT/triangle_3_8.pcf XYZIRGB 8 0.001
../BUILD/enea --verbose --read-e57-cloud IN/triangle_3.e57 --write-pcf-cloud OUT/triangle_3_32.pcf XYZIRGB 32 0
../BUILD/enea --verbose --read-e57-cloud IN/cube.e57 --write-xyz-cloud OUT/cube.xyz XYZ
../BUILD/enea --verbose --read-e57-cloud IN/cube.e57 --write-pcf-cloud OUT/cube_8.pcf XYZIRGB 8 0.001
../BUILD/enea --verbose --read-e57-cloud IN/cube.e57 --write-pcf-cloud OUT/cube_32.pcf XYZIRGB 32 0
../BUILD/enea --verbose --read-e57-cloud IN/cube_2.e57 --write-xyz-cloud OUT/cube_2.xyz XYZ
../BUILD/enea --verbose --read-e57-cloud IN/cube_2.e57 --write-pcf-cloud OUT/cube_2_8.pcf XYZIRGB 8 0.001
../BUILD/enea --verbose --read-e57-cloud IN/cube_2.e57 --write-pcf-cloud OUT/cube_2_32.pcf XYZIRGB 32 0
../BUILD/enea --verbose --read-e57-cloud IN/cube_3.e57 --write-xyz-cloud OUT/cube_3.xyz XYZ
../BUILD/enea --verbose --verbose --read-e57-cloud IN/cube_3.e57 --write-pcf-cloud OUT/cube_3_8.pcf XYZIRGB 8 0.001
../BUILD/enea --read-e57-cloud IN/cube_3.e57 --write-pcf-cloud OUT/cube_3_32.pcf XYZIRGB 32 0
../BUILD/enea --read-e57-cloud IN/teapot.e57 --write-xyz-cloud OUT/teapot.xyz XYZ
../BUILD/enea --read-e57-cloud IN/teapot.e57 --write-pcf-cloud OUT/teapot_8.pcf XYZIRGB 8 0.001
../BUILD/enea --read-e57-cloud IN/teapot.e57 --write-pcf-cloud OUT/teapot_32.pcf XYZIRGB 32 0
../BUILD/enea --read-e57-cloud IN/bunny.e57 --write-xyz-cloud OUT/bunny.xyz XYZ
../BUILD/enea --read-e57-cloud IN/bunny.e57 --write-pts-cloud OUT/bunny.pts XYZIRGB
../BUILD/enea --read-e57-cloud IN/bunny.e57 --write-pcf-cloud OUT/bunny_8.pcf XYZIRGB 8 0.001
../BUILD/enea --read-e57-cloud IN/bunny.e57 --write-pcf-cloud OUT/bunny_32.pcf XYZIRGB 32 0
../BUILD/enea --position-rotation 0 0 -45 --read-e57-cloud IN/bunny.e57 --write-xyz-cloud OUT/rotated_bunny.xyz XYZ
