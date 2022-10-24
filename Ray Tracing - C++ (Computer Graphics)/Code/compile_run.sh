#! /bin/bash
# rm -r build
mkdir build
cd build
cmake ../
make

./pathtracer -t 4 -s 2048 -a 64 0.05 -l 1 -m 5 -r 480 360 -f bunny.png ../dae/sky/CBbunny.dae
# ./pathtracer -t 4 -s 64 -l 32 -f dragon_64_32.png -r 480 480 ../dae/sky/dragon.dae
# ./pathtracer -t 4 -s 16 -l 16 -H -f bunny_1_1.png -r 480 360 ../dae/sky/CBbunny.dae
# ./pathtracer -t 4 -s 64 -l 32 -H -r 100 100 ../dae/sky/CBbunny.dae
# ./pathtracer -t 4 -s 64 -l 32 -H -f CBbunny_H_64_32.png -r 480 360 ../dae/sky/CBbunny.dae
# ./pathtracer -t 4 -s 16 -l 8 -m 6 -H -f CBbunny_16_8.png -r 480 360 ../dae/sky/CBbunny.dae
# ./pathtracer -r 800 600 -f CBlucy.png ../dae/sky/CBlucy.dae 
# ./pathtracer -t 4 -r 800 600 -f banana.png ../dae/keenan/banana.dae
# ./pathtracer -t 4 -r 800 600 -f building.png ../dae/keenan/building.dae
# ./pathtracer -t 4 -r 800 600 -f beast.png ../dae/meshedit/beast.dae
# ./pathtracer -t 4 -r 800 600 -f beast.png ../dae/meshedit/beetle.dae
# ./pathtracer -t 4 -r 800 600 -f maxplanck.png ../dae/meshedit/maxplanck.dae
# ./pathtracer -t 4 -r 800 600 -f cube.png ../dae/simple/cube.dae
# ./pathtracer -t 4 -r 800 600 -f plane.png ../dae/simple/plane.dae
# ./pathtracer -t 4 -r 800 600 -f bunny.png ../dae/sky/bunny.dae
# ./pathtracer -r 800 600 -f CBempty.png ../dae/sky/CBempty.dae
# ./pathtracer -r 800 600 -f banana.png ../dae/keenan/banana.dae
# ./pathtracer -r 800 600 -f CBspheres.png ../dae/sky/CBspheres_lambertian.dae
# ./pathtracer -t 4 -r 800 600 -f cow.png ../dae/meshedit/cow.dae
# 
# ./pathtracer -r 800 600 ../dae/sky/CBbunny.dae
# /bin/bash compile_run.sh