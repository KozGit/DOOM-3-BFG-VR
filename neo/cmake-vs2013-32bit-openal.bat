cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 12 2013" -DCMAKE_INSTALL_PREFIX=../bin/win8-32 -DOPENAL=ON ../neo
pause