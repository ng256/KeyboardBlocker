@echo off
setlocal

REM ============================================================================ 
REM Keyboard Blocker - Windows application to block keyboard input
REM Copyright (c) 2025 Pavel Bashkardin

REM This file is part of Keyboard Blocker project.

REM Permission is hereby granted, free of charge, to any person obtaining a copy
REM of this software and associated documentation files (the "Software"), to deal
REM in the Software without restriction, including without limitation the rights
REM to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
REM copies of the Software, and to permit persons to whom the Software is
REM furnished to do so, subject to the following conditions:

REM The above copyright notice and this permission notice shall be included in all
REM copies or substantial portions of the Software.

REM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
REM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
REM SOFTWARE.

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
set "FILE1=keyblock.exe"
set "FILE2=install.bat"
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