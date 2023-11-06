cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_SYSTEM_VERSION="10.0.10240.0" -DCMAKE_INSTALL_PREFIX=../bin/win8-64 -DCMAKE_BUILD_TYPE=Debug ../neo
pause