cd ./IDLs
MegrezC -c benchmark.mgz
cd ../
g++ bm_megrez.cc -o bm_megrez -I ./IDLs/ -I ../

read -p " "