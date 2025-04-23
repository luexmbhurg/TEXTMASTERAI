@echo off
echo ===== Building LessonDecoder Windows Application =====

REM Kill any running instances of the application
taskkill /F /IM LessonDecoder.exe 2>nul

REM Set Qt paths
set "QT_PATH=C:\Qt\6.6.3\msvc2019_64"
set "PATH=%QT_PATH%\bin;%PATH%"

REM Check for prerequisites
where /q cmake
IF ERRORLEVEL 1 (
    echo CMake not found. Please install CMake first.
    exit /b 1
)

REM Check for Python 3.11
where /q py -3.11
IF ERRORLEVEL 1 (
    echo Python 3.11 not found. Please install Python 3.11 first.
    exit /b 1
)

REM Check for Qt6
IF NOT EXIST "%QT_PATH%\bin\qmake.exe" (
    echo Qt6 not found at %QT_PATH%\bin\qmake.exe
    exit /b 1
)

REM Install required Python packages
echo Installing Python dependencies...
py -3.11 -m pip install --upgrade pip
py -3.11 -m pip install spacy transformers torch numpy
py -3.11 -m spacy download en_core_web_sm

REM Clean and create build directory
IF EXIST build (
    echo Cleaning build directory...
    rmdir /S /Q build
)
mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_PREFIX_PATH="%QT_PATH%" -DQt6_DIR="%QT_PATH%\lib\cmake\Qt6" -DCMAKE_BUILD_TYPE=Release
IF ERRORLEVEL 1 (
    echo CMake configuration failed
    cd ..
    exit /b 1
)

REM Build the project
echo Building the project...
cmake --build . --config Release
IF ERRORLEVEL 1 (
    echo Build failed
    cd ..
    exit /b 1
)

echo Copying Python resources...
if not exist "Release\resources" mkdir "Release\resources"
xcopy /Y /I "..\src\resources\nlp_processor.py" "Release\resources\"
IF ERRORLEVEL 1 (
    echo Failed to copy Python resources
    cd ..
    exit /b 1
)

echo ===== Build Complete =====
echo Executable is located in: %CD%\Release\LessonDecoder.exe
cd ..\

pause
