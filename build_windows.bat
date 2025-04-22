
@echo off
echo ===== Building TextMaster Windows Application =====

REM Check for prerequisites
where /q cmake
IF ERRORLEVEL 1 (
    echo CMake not found. Please install CMake first.
    exit /b 1
)

where /q python
IF ERRORLEVEL 1 (
    echo Python not found. Please install Python first.
    exit /b 1
)

REM Install required Python packages
echo Installing Python dependencies...
python -m pip install --upgrade pip
python -m pip install spacy transformers torch numpy pyinstaller
python -m spacy download en_core_web_sm

REM Create build directory
IF NOT EXIST build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake ..

REM Build the project
echo Building the project...
cmake --build . --config Release

REM Create Windows executable
echo Creating Windows executable...
cd Release
pyinstaller --onefile --windowed --name TextMaster ^
    --add-data "resources/nlp_processor.py;resources/" ^
    --icon="../../resources/app_icon.ico" ^
    TextMaster.exe

echo ===== Build Complete =====
echo Executable is located in: %CD%\dist\TextMaster.exe
cd ..\..\

pause
