@echo off
setlocal enabledelayedexpansion

echo Building llama.cpp...
echo ===================

cd llama.cpp\llama.cpp-master

:: Create build directory
if exist "build" (
    rd /s /q build
)
mkdir build
cd build

:: Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DLLAMA_STATIC=ON ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DLLAMA_NATIVE=OFF ^
    -DLLAMA_AVX=OFF ^
    -DLLAMA_AVX2=ON ^
    -DLLAMA_FMA=ON ^
    -DLLAMA_F16C=ON ^
    -DLLAMA_CURL=OFF ^
    -DBUILD_EXAMPLES=ON

if %errorLevel% neq 0 (
    echo Error: CMake configuration failed
    cd ..\..\..
    exit /b 1
)

:: Build the project
cmake --build . --config Release --target llama-cli

if %errorLevel% neq 0 (
    echo Error: Build failed
    cd ..\..\..
    exit /b 1
)

cd ..\..\..

echo Build completed successfully! 