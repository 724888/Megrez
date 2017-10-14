cd ./IDLs
./MegrezC -c benchmark.mgz
./protoc benchmark.proto --cpp_out=.
cd ../
g++ bm_megrez.cc -o bm_megrez -I ./IDLs/ 
cd ./build
cmake ../
read -p " "