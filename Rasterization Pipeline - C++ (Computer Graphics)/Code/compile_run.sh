#! /bin/bash
mkdir build
cd build
cmake ../
make

# rm ../mark.txt

./draw ../svg/basic/test4.svg
./draw ../docs/my_robot.svg
./draw ../docs/barycentric.svg
./draw ../svg/basic/test7.svg
./draw ../svg/texmap/test1.svg
./draw ../docs/my_png.svg

# cd ../tests/outputs
# /bin/python3 test_all.py >> mark.txt

# rm -r ../../build

# /bin/bash compile_run.sh
