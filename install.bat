@echo off

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

REM Description: Installation script for Keyboard Blocker
REM ============================================================================

REM --------------------------------------------
REM Get current folder
REM --------------------------------------------

set "APPDIR=%~dp0"
if "%APPDIR:~-1%"=="\" set "APPDIR=%APPDIR:~0,-1%"

set "APP_NAME=Keyboard Blocker"
set "PRJ_NAME=Keyblock"

REM --------------------------------------------
REM Initialize registry
REM --------------------------------------------

reg delete "HKCU\Software\Keyblock" /f >nul 2>&1
reg add "HKCU\Software\Keyblock" /f >nul
reg add "HKCU\Software\Keyblock" ^
 /v "State" ^
 /t REG_DWORD ^
 /d 0 ^
 /f >nul

REM --------------------------------------------
REM Reset startup entry
REM --------------------------------------------

reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" ^
 /v "Keyboard Blocker" ^
 /f >nul 2>&1

reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" ^
 /v "Keyboard Blocker" ^
 /t REG_SZ ^
 /d "\"%APPDIR%\keyblock.exe\"" ^
 /f >nul

REM --------------------------------------------
REM Register uninstall in "Programs and Features"
REM --------------------------------------------
set "UNREGKEY=HKCU\Software\Microsoft\Windows\CurrentVersion\Uninstall\Keyboard Blocker"
reg add "%UNREGKEY%" /f >nul
reg add "%UNREGKEY%" /v "DisplayName" /t REG_SZ /d "Keyboard Blocker" /f >nul
reg add "%UNREGKEY%" /v "UninstallString" /t REG_SZ /d "\"%APPDIR%\uninstall.exe\"" /f >nul
reg add "%UNREGKEY%" /v "Publisher" /t REG_SZ /d "Pavel Bashkardin" /f >nul

if exist "%APPDIR%\icon32.ico" (
    reg add "%UNREGKEY%" /v "DisplayIcon" /t REG_SZ /d "%APPDIR%\icon32.ico" /f >nul
)

reg add "%UNREGKEY%" /v "NoModify" /t REG_DWORD /d 1 /f >nul
reg add "%UNREGKEY%" /v "NoRepair" /t REG_DWORD /d 1 /f >nul

REM --------------------------------------------
REM Create shortcuts
REM --------------------------------------------

set "LINK_NAME=Keyboard Blocker"
set "TARGET=%APPDIR%\keyblock.exe"

REM Desktop
set "DESKTOP=%USERPROFILE%\Desktop\%LINK_NAME%.lnk"

REM Start Menu
set "STARTMENU=%APPDATA%\Microsoft\Windows\Start Menu\Programs\%LINK_NAME%.lnk"

REM Create VBScript
set "VBS=%TEMP%\create_shortcut.vbs"

echo Set oWS = WScript.CreateObject("WScript.Shell") > "%VBS%"
echo sLinkFile = WScript.Arguments(0) >> "%VBS%"
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> "%VBS%"
echo oLink.TargetPath = WScript.Arguments(1) >> "%VBS%"
echo oLink.WorkingDirectory = WScript.Arguments(2) >> "%VBS%"
echo oLink.IconLocation = WScript.Arguments(1) ^& ",0" >> "%VBS%"
echo oLink.Save >> "%VBS%"

REM Create Desktop shortcut
cscript //nologo "%VBS%" "%DESKTOP%" "%TARGET%" "%APPDIR%"

REM Create Start Menu shortcut
cscript //nologo "%VBS%" "%STARTMENU%" "%TARGET%" "%APPDIR%"

del "%VBS%" >nul 2>&1

REM --------------------------------------------
REM Run program and self delete
REM --------------------------------------------
start "" "%APPDIR%\keyblock.exe"
start "" cmd /c del "%~f0"
exit