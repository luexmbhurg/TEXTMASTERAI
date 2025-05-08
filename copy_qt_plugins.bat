@echo off
set QT_DIR=C:\Qt\6.6.3\msvc2019_64
set TARGET_DIR=build\Release

mkdir %TARGET_DIR%\platforms
mkdir %TARGET_DIR%\styles
mkdir %TARGET_DIR%\imageformats

copy %QT_DIR%\plugins\platforms\qwindows.dll %TARGET_DIR%\platforms\
copy %QT_DIR%\plugins\styles\qwindowsvistastyle.dll %TARGET_DIR%\styles\
copy %QT_DIR%\plugins\imageformats\*.dll %TARGET_DIR%\imageformats\

echo Qt plugins copied successfully! 