echo -------------------------Start processing--------------------------
echo
echo ---------------------Start generating Makefile---------------------
cd ./build
cmake -G "Unix Makefiles" ../
echo
echo ---------------------------Start building--------------------------
make
echo
echo -------------------------------Done--------------------------------
echo
read -p "Press Any Keys to Exit......"