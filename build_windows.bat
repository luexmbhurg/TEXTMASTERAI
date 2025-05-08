@echo off
echo Building TextMaster...

:: Create build directory if it doesn't exist
if not exist "build" mkdir build
cd build

:: Configure with CMake for Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

:: Build the project
cmake --build . --config Release

:: Check if build was successful
if exist "Release\TextMaster.exe" (
    echo Build completed successfully!
    echo Executable location: build\Release\TextMaster.exe
) else (
    echo Error: Build failed - executable not found
)

cd ..
