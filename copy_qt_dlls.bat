@echo off
set QT_DIR=C:\Qt\6.6.3\msvc2019_64\bin
set TARGET_DIR=build\Release

copy %QT_DIR%\Qt6Core.dll %TARGET_DIR%\
copy %QT_DIR%\Qt6Gui.dll %TARGET_DIR%\
copy %QT_DIR%\Qt6Widgets.dll %TARGET_DIR%\
copy %QT_DIR%\Qt6Network.dll %TARGET_DIR%\
copy %QT_DIR%\Qt6Svg.dll %TARGET_DIR%\
copy %QT_DIR%\Qt6Pdf.dll %TARGET_DIR%\

echo Qt DLLs copied successfully! 