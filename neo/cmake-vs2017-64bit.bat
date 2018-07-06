cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX=../bin/win8-64 ../neo
pause