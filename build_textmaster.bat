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
if not exist "%VS_PATH%\Common7\IDE\devenv.com" (
    echo %YELLOW%Warning: Visual Studio 2022 not found at %VS_PATH%%RESET%
    echo Please install Visual Studio 2022 or update the VS_PATH in this script
)

:: Set up environment
echo Setting up environment...
set "PATH=%QT_PATH%\bin;%PATH%"

:: Create build directory
echo Creating build directory...
if not exist "build" mkdir build

:: Clean existing build directory
echo Cleaning existing build directory...
cd build
if exist "CMakeCache.txt" del /q CMakeCache.txt
if exist "CMakeFiles" rmdir /s /q CMakeFiles

:: Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_PREFIX_PATH="%QT_PATH%" -DCMAKE_BUILD_TYPE=Release

:: Build the project
echo Building TextMaster...
cmake --build . --config Release

:: Create required directories
echo Creating required directories...
if not exist "Release\models" mkdir "Release\models"
if not exist "Release\styles" mkdir "Release\styles"
if not exist "Release\resources" mkdir "Release\resources"
if not exist "Release\platforms" mkdir "Release\platforms"
if not exist "Release\styles" mkdir "Release\styles"
if not exist "Release\imageformats" mkdir "Release\imageformats"

:: Copy model files
echo Copying model files...
if exist "..\models\tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf" (
    copy /y "..\models\tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf" "Release\models\"
) else (
    echo %YELLOW%Warning: TinyLlama model file not found%RESET%
)

:: Copy resources
echo Copying resources...
xcopy /y /s "..\resources\*" "Release\resources\"
xcopy /y /s "..\forms\*" "Release\forms\"

:: Create README
echo Creating README.txt...
echo TextMaster Application > "Release\README.txt"
echo ===================== >> "Release\README.txt"
echo. >> "Release\README.txt"
echo This is the TextMaster application, a tool for analyzing text using TinyLlama. >> "Release\README.txt"
echo. >> "Release\README.txt"
echo Requirements: >> "Release\README.txt"
echo - Windows 10 or later >> "Release\README.txt"
echo - 4GB RAM minimum >> "Release\README.txt"
echo. >> "Release\README.txt"
echo For support, please visit: https://github.com/yourusername/TextMaster >> "Release\README.txt"

:: Copy Qt plugins
echo Copying Qt plugins...
copy /y "%QT_PATH%\plugins\platforms\qwindows.dll" "Release\platforms\"
copy /y "%QT_PATH%\plugins\styles\qwindowsvistastyle.dll" "Release\styles\"
copy /y "%QT_PATH%\plugins\imageformats\qgif.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qicns.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qico.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qjpeg.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qsvg.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qtga.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qtiff.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qwbmp.dll" "Release\imageformats\"
copy /y "%QT_PATH%\plugins\imageformats\qwebp.dll" "Release\imageformats\"
copy /y "%QT_PATH%\bin\Qt6Core.dll" "Release\"

:: Copy style files
echo Copying style files...
xcopy /y /s "..\resources\styles\*.qss" "Release\resources\styles\"

if %errorLevel% equ 0 (
    echo %GREEN%Build completed successfully%RESET%
    echo.
    echo Executable location: build\Release\TextMaster.exe
    echo.
    echo %YELLOW%Note: Make sure to place the TinyLlama model file in the models directory%RESET%
    echo.
    echo %GREEN%Build process completed%RESET%
) else (
    echo %RED%Error: Build failed%RESET%
    exit /b 1
)

cd .. 