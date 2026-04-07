@echo off
setlocal

REM ============================================================================ 
REM Keyboard Blocker - Windows application to block keyboard input
REM Copyright (c) 2025 Pavel Bashkardin

REM This file is part of Keyboard Blocker project, released under MIT license.

REM Description: Build SFX archive for Keyboard Blocker
REM ============================================================================

echo.
echo Starting build SFX archive...

REM Path to WinRAR  (adjust if necessary)
set "RAR=C:\Program Files\WinRAR\winrar.exe"

REM Output file
set "OUT=Setup.exe"

REM SFX script
set "COMMENT=sfx.ini"

REM Required files
set "FILE0=keyblock.exe"
set "FILE1=keyblock.tmp"
set "FILE2=install.js"
set "FILE3=uninstall.exe"
set "ICON=icon32.ico"
set "LOGO=sfx.bmp"

REM --------------------------------------------
REM Check WinRAR
REM --------------------------------------------
echo Checking WinRAR...

if not exist "%RAR%" (
    echo WinRAR not found: %RAR%
    echo Set correct path to winrar.exe
    goto :error
)

REM --------------------------------------------
REM Prepare temporary copy (avoid file lock issue)
REM --------------------------------------------
echo Preparing temporary executable...
copy /Y "%FILE0%" "%FILE1%" >nul
if errorlevel 1 (
    echo Failed to create keyblock.tmp
    goto :error
)

REM --------------------------------------------
REM Check required files
REM --------------------------------------------
echo Checking input files...

if not exist "%FILE1%" (
    echo Missing file: %FILE1%
    goto :error
)

if not exist "%FILE2%" (
    echo Missing file: %FILE2%
    goto :error
)

if not exist "%FILE3%" (
    echo Missing file: %FILE3%
    goto :error
)

if not exist "%COMMENT%" (
    echo Missing SFX script: %COMMENT%
    goto :error
)

if not exist "%ICON%" (
    echo Icon not found: %ICON%
    echo Default icon will be used
    set "ICON_PARAM="
) else (
    set "ICON_PARAM=-iicon%ICON%"
)

if not exist "%LOGO%" (
    echo Icon not found: %LOGO%
    echo Default icon will be used
    set "LOGO_PARAM="
) else (
    set "LOGO_PARAM=-iimg%LOGO%"
)

REM --------------------------------------------
REM Remove old build
REM --------------------------------------------
if exist "%OUT%" (
    echo Removing old build...
    del "%OUT%"
    if exist "%OUT%" (
        echo Failed to delete old file: %OUT%
        goto :error
    )
)

REM --------------------------------------------
REM Build archive
REM --------------------------------------------
echo Creating SFX archive...

"%RAR%" a -r -sfx -iadm -z"%COMMENT%" %ICON_PARAM% %LOGO_PARAM% "%OUT%" %ICON% "%FILE1%" "%FILE2%" "%FILE3%"
if errorlevel 1 (
    echo WinRAR failed with code %ERRORLEVEL%
    goto :error
)

REM --------------------------------------------
REM Cleanup temporary files
REM --------------------------------------------
echo Cleaning temporary files...
if exist "%FILE1%" del "%FILE1%"

REM --------------------------------------------
REM Verify result
REM --------------------------------------------
if not exist "%OUT%" (
    echo Output file was not created
    goto :error
)

echo Build completed: %OUT%
goto :end

REM --------------------------------------------
REM Error handler
REM --------------------------------------------
:error
echo Build failed.
pause
exit /b 1

:end
echo Done.
pause
exit /b 0
