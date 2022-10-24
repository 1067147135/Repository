#! /bin/bash
# rm -r build
mkdir build
cd build
cmake ../
make


# ./pathtracer -t 4 -s 64 -l 4 -m 0 -f CBspheres_0.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 1 -f CBspheres_1.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 2 -f CBspheres_2.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 3 -f CBspheres_3.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 4 -f CBspheres_4.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 5 -f CBspheres_5.png -r 480 360 ../dae/sky/CBspheres.dae
# ./pathtracer -t 4 -s 64 -l 4 -m 100 -f CBspheres_100.png -r 480 360 ../dae/sky/CBspheres.dae

./pathtracer -t 4 -s 4 -l 64 -e ../exr/field.exr -f field_bunny_unlit_uniform.png ../dae/sky/bunny_unlit.dae
./pathtracer -t 4 -s 4 -l 64 -e ../exr/field.exr -f field_banana_uniform.png ../dae/keenan/banana.dae

# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.05 -d 1.0 -f field_bunny_unlit_0.05_1.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.05 -d 3.0 -f field_bunny_unlit_0.05_3.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.05 -d 5.0 -f field_bunny_unlit_0.05_5.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.05 -d 7.0 -f field_bunny_unlit_0.05_7.0.png ../dae/sky/bunny_unlit.dae

# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.01 -d 4.0 -f field_bunny_unlit_0.01_4.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.03 -d 4.0 -f field_bunny_unlit_0.03_4.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.05 -d 4.0 -f field_bunny_unlit_0.05_4.0.png ../dae/sky/bunny_unlit.dae
# ./pathtracer -t 4 -s 64 -l 4 -e ../exr/field.exr -b 0.07 -d 4.0 -f field_bunny_unlit_0.07_4.0.png ../dae/sky/bunny_unlit.dae
# /bin/bash compile_run.sh