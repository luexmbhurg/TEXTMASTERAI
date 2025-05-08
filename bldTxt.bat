@echo off
setlocal enabledelayedexpansion

echo TextMaster Build Script
echo =====================

:: Set error color
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RESET=[0m"

:: Check for admin privileges
net session >nul 2>&1
if %errorLevel% == 0 (
    echo %GREEN%Running with administrator privileges%RESET%
) else (
    echo %YELLOW%Warning: Not running with administrator privileges%RESET%
)

:: Check for required tools
echo Checking required tools...
where cmake >nul 2>&1
if %errorLevel% neq 0 (
    echo %RED%Error: CMake not found. Please install CMake.%RESET%
    exit /b 1
)

:: Setup vcpkg if not present
if not exist "vcpkg\vcpkg.exe" (
    echo Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if !errorLevel! neq 0 (
        echo %RED%Error: Failed to clone vcpkg%RESET%
        exit /b 1
    )
    cd vcpkg
    call bootstrap-vcpkg.bat
    if !errorLevel! neq 0 (
        echo %RED%Error: Failed to bootstrap vcpkg%RESET%
        cd ..
        exit /b 1
    )
    cd ..
)

:: Install required packages
echo Installing required packages...
vcpkg\vcpkg install llama-cpp:x64-windows
if %errorLevel% neq 0 (
    echo %RED%Error: Failed to install llama-cpp%RESET%
    exit /b 1
)

:: Integrate vcpkg
vcpkg\vcpkg integrate install
if %errorLevel% neq 0 (
    echo %RED%Error: Failed to integrate vcpkg%RESET%
    exit /b 1
)

:: Clean build directory
if exist build (
    echo Cleaning build directory...
    rd /s /q build
    if !errorLevel! neq 0 (
        echo %YELLOW%Warning: Failed to clean build directory%RESET%
    )
)

:: Create fresh build directory
mkdir build
cd build

:: Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
if %errorLevel% neq 0 (
    echo %RED%Error: CMake configuration failed%RESET%
    cd ..
    exit /b 1
)

:: Build the project
echo Building TextMaster...
cmake --build . --config Release
if %errorLevel% neq 0 (
    echo %RED%Error: Build failed%RESET%
    cd ..
    exit /b 1
)

:: Create models directory
if not exist "Release\models" (
    mkdir "Release\models"
)

:: Copy model readme
copy ..\models\README.txt Release\models\ >nul 2>&1

:: Check if build was successful
if exist "Release\TextMaster.exe" (
    echo %GREEN%Build completed successfully!%RESET%
    echo Executable location: build\Release\TextMaster.exe
) else (
    echo %RED%Error: Build failed - executable not found%RESET%
    cd ..
    exit /b 1
)

cd ..

:: Check for Mistral model
if not exist "models\mistral-7b-v0.1.Q4_K_M.gguf" (
    echo %YELLOW%Warning: Mistral model not found in models directory%RESET%
    echo Please download mistral-7b-v0.1.Q4_K_M.gguf and place it in the models directory
)

echo.
echo %GREEN%Build process completed%RESET%
endlocal 