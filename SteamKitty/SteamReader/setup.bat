@echo off
echo Setting up Steam Reader C++ Project...

:: Create project directory structure
mkdir steam_reader
cd steam_reader
mkdir src
mkdir include
mkdir lib

:: Download nlohmann JSON header
echo Downloading nlohmann/json library...
powershell -Command "Invoke-WebRequest -Uri 'https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp' -OutFile 'include/json.hpp'"

echo.
echo Project structure created!
echo.
echo To build:
echo 1. Open in Visual Studio or use CMake
echo 2. Add wininet.lib to linker dependencies
echo 3. Include the nlohmann/json.hpp header
echo.
echo Alternatively, compile directly with:
echo g++ -std=c++17 steam_reader_console.cpp -lwininet -o steam_reader.exe
echo.
pause