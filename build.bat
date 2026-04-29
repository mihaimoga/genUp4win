@echo off
mkdir build
cd build
cmake -G "Visual Studio 18 2026" ..
cmake --build . --config Release
cd ..
pause
