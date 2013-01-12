@echo OFF
tasklist /FI "IMAGENAME eq 5 Minute Break Dev.exe"  |findstr /I /C:"5 minute break dev.exe" > NUL
if %ERRORLEVEL% equ 0 taskkill /F /IM "5 Minute Break Dev.exe"

