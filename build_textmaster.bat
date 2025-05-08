@echo off
setlocal enabledelayedexpansion

echo TextMaster Build Script
echo =====================

:: Set colors for output
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RESET=[0m"

:: Check for required tools
echo Checking required tools...

:: Check for CMake
where cmake >nul 2>&1
if %errorLevel% neq 0 (
    echo %RED%Error: CMake not found. Please install CMake.%RESET%
    exit /b 1
)

:: Check for Qt6
set "QT_PATH=C:\Qt\6.6.3\msvc2019_64"
if not exist "%QT_PATH%\bin\qmake.exe" (
    echo %YELLOW%Warning: Qt6 not found at %QT_PATH%%RESET%
    echo Please install Qt6 or update the QT_PATH in this script
    exit /b 1
)

:: Check for Visual Studio
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
if not exist "%VS_PATH%\Common7\IDE\devenv.exe" (
    echo %YELLOW%Warning: Visual Studio 2022 not found at %VS_PATH%%RESET%
    echo Please install Visual Studio 2022 or update the VS_PATH in this script
    exit /b 1
)

:: Set environment variables
echo Setting up environment...
set "PATH=%QT_PATH%\bin;%PATH%"
set "CMAKE_PREFIX_PATH=%QT_PATH%"

:: Create build directory
echo Creating build directory...
if exist "build" (
    echo Cleaning existing build directory...
    rd /s /q build
)
mkdir build
cd build

:: Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%QT_PATH%" ^
    -DCMAKE_INSTALL_PREFIX=install

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

:: Create required directories
echo Creating required directories...
if not exist "Release\models" mkdir "Release\models"
if not exist "Release\resources" mkdir "Release\resources"

:: Copy model file
echo Copying model file...
if exist "..\models\mistral-7b-v0.1.Q4_K_M.gguf" (
    copy "..\models\mistral-7b-v0.1.Q4_K_M.gguf" "Release\models\"
    echo Model file copied successfully
) else (
    echo %YELLOW%Warning: Model file not found at ..\models\mistral-7b-v0.1.Q4_K_M.gguf%RESET%
    echo Please download the model file and place it in the models directory
)

:: Copy resources
echo Copying resources...
xcopy /E /I /Y "..\resources\*" "Release\resources\"
xcopy /E /I /Y "..\forms\*" "Release\forms\"

:: Create model readme
echo Creating model readme...
echo Place your GGUF model files in this directory. > "Release\models\README.txt"
echo. >> "Release\models\README.txt"
echo Download models from: >> "Release\models\README.txt"
echo - https://huggingface.co/TheBloke/Mistral-7B-v0.1-GGUF >> "Release\models\README.txt"
echo. >> "Release\models\README.txt"
echo Recommended model: >> "Release\models\README.txt"
echo - mistral-7b-v0.1.Q4_K_M.gguf >> "Release\models\README.txt"
echo. >> "Release\models\README.txt"
echo These files are large (4GB) and not included in the repository. >> "Release\models\README.txt"

:: Check if build was successful
if exist "Release\TextMaster.exe" (
    echo %GREEN%Build completed successfully!%RESET%
    echo.
    echo Executable location: build\Release\TextMaster.exe
    echo.
    echo %YELLOW%Note: Make sure to place the Mistral model file in the models directory%RESET%
) else (
    echo %RED%Error: Build failed - executable not found%RESET%
    cd ..
    exit /b 1
)

cd ..
echo.
echo %GREEN%Build process completed%RESET%
endlocal 