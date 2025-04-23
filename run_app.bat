@echo off 
setlocal 
cd /d "C:\Projects\audio-to-brief-notes\build\Release" 
echo Starting LessonDecoder from C:\Projects\audio-to-brief-notes\build\Release... 
dir 
"C:\Projects\audio-to-brief-notes\build\Release\LessonDecoder.exe" > "C:\Projects\audio-to-brief-notes\logs\app.log" 2>&1 
type "C:\Projects\audio-to-brief-notes\logs\app.log" 
echo. 
echo Log file saved to: "C:\Projects\audio-to-brief-notes\logs\app.log" 
pause 
