@echo off
set "VCRUNTIME_DIR=C:\Windows\System32"
set "TARGET_DIR=build\Release"
 
copy "%VCRUNTIME_DIR%\MSVCP140.dll" "%TARGET_DIR%"
copy "%VCRUNTIME_DIR%\VCRUNTIME140.dll" "%TARGET_DIR%"
copy "%VCRUNTIME_DIR%\VCRUNTIME140_1.dll" "%TARGET_DIR%" 