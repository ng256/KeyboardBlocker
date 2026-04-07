@echo off
setlocal enabledelayedexpansion

REM ============================================================================ 
REM Keyboard Blocker - Windows application to block keyboard input
REM Copyright (c) 2025 Pavel Bashkardin

REM This file is part of Keyboard Blocker project, released under MIT license.

REM Description: Build script for Windows
REM ============================================================================

REM ------------------------------------------------------------
REM Configuration
REM ------------------------------------------------------------
set PROJECT_NAME=keyblock
set KEYBLOCK_RC=resource.rc
set KEYBLOCK_C=keyblock.c

REM Uninstaller
set UNINSTALL_NAME=uninstall
set UNINSTALL_RC=uninstall.rc
set UNINSTALL_C=uninstall.c

REM Derive object file names from source files
set KEYBLOCK_RC_OBJ=%KEYBLOCK_RC:.rc=.o%
set KEYBLOCK_C_OBJ=%KEYBLOCK_C:.c=.o%
set KEYBLOCK_EXE=%PROJECT_NAME%.exe

set UNINSTALL_RC_OBJ=%UNINSTALL_RC:.rc=.rc.o%
set UNINSTALL_OBJ=%UNINSTALL_C:.c=.o%
set UNINSTALL_EXE=%UNINSTALL_NAME%.exe

REM Path to GCC bin folder (adjust if necessary)
set PATH=C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin;%PATH%

REM --------------------------------------------
REM Check required files
REM --------------------------------------------
echo Checking input files...

if not exist "%KEYBLOCK_RC%" (
    echo Missing file: %KEYBLOCK_RC%
    goto :error
)

if not exist "%KEYBLOCK_C%" (
    echo Missing file: %KEYBLOCK_C%
    goto :error
)

if not exist "%UNINSTALL_C%" (
    echo Missing file: %UNINSTALL_C%
    goto :error
)

REM ------------------------------------------------------------
REM Clean previous build files
REM ------------------------------------------------------------
echo Cleaning previous build files...
if exist %KEYBLOCK_C_OBJ% del %KEYBLOCK_C_OBJ%
if exist %KEYBLOCK_RC_OBJ% del %KEYBLOCK_RC_OBJ%
if exist %KEYBLOCK_EXE% del %KEYBLOCK_EXE%
if exist %UNINSTALL_OBJ% del %UNINSTALL_OBJ%
if exist %UNINSTALL_EXE% del %UNINSTALL_EXE%

REM ------------------------------------------------------------
REM Compile resources
REM ------------------------------------------------------------
echo Compiling resources...
windres -i %KEYBLOCK_RC% -o %KEYBLOCK_RC_OBJ% --input-format=rc -O coff -F pe-i386
windres -i %UNINSTALL_RC% -o %UNINSTALL_RC_OBJ% --input-format=rc -O coff -F pe-i386
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Compile C source for main app
REM ------------------------------------------------------------
echo Compiling %KEYBLOCK_C%...
gcc -m32 -Os -s -static -static-libgcc -ffunction-sections -fdata-sections -mwindows -I. -c %KEYBLOCK_C% -o %KEYBLOCK_C_OBJ%
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Compile C source for uninstaller
REM ------------------------------------------------------------
echo Compiling %UNINSTALL_C%...
gcc -m32 -Os -s -static -static-libgcc -ffunction-sections -fdata-sections -mwindows -I. -c %UNINSTALL_C% -o %UNINSTALL_OBJ%
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Link main app
REM ------------------------------------------------------------
echo Linking %KEYBLOCK_EXE%...
gcc -m32 -Os -s -static -static-libgcc -ffunction-sections -fdata-sections -mwindows %KEYBLOCK_C_OBJ% %KEYBLOCK_RC_OBJ% -o %KEYBLOCK_EXE% -Wl,--gc-sections
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Link uninstaller
REM ------------------------------------------------------------
echo Linking %UNINSTALL_EXE%...
gcc -m32 -Os -s -static -static-libgcc -ffunction-sections -fdata-sections -mwindows %UNINSTALL_OBJ% %UNINSTALL_RC_OBJ% -o %UNINSTALL_EXE% -Wl,--gc-sections -lshlwapi
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Stripping
REM ------------------------------------------------------------
echo Stripping executables...
strip %KEYBLOCK_EXE%
strip %UNINSTALL_EXE%
if errorlevel 1 goto error

REM ------------------------------------------------------------
REM Show imported DLLs
REM ------------------------------------------------------------
echo Checking dependencies for main exe:
objdump -p %KEYBLOCK_EXE% | find "DLL Name"
echo Checking dependencies for uninstaller:
objdump -p %UNINSTALL_EXE% | find "DLL Name"

REM ------------------------------------------------------------
REM Optional UPX compression
REM ------------------------------------------------------------
where upx >nul 2>nul
if errorlevel 1 (
    echo UPX not found, skipping compression.
) else (
    echo Applying UPX compression...
    upx --best %KEYBLOCK_EXE%
    upx --best %UNINSTALL_EXE%
    if errorlevel 1 (
        echo UPX compression failed.
    ) else (
        echo UPX compression completed.
    )
)

REM ------------------------------------------------------------
REM Clean build files
REM ------------------------------------------------------------
echo Cleaning object files...
if exist %KEYBLOCK_C_OBJ% del %KEYBLOCK_C_OBJ%
if exist %KEYBLOCK_RC_OBJ% del %KEYBLOCK_RC_OBJ%
if exist %UNINSTALL_RC_OBJ% del %UNINSTALL_RC_OBJ%
if exist %UNINSTALL_OBJ% del %UNINSTALL_OBJ%

REM ------------------------------------------------------------
REM Build SFX
REM ------------------------------------------------------------

buildsfx.bat

echo Done. Output files: %KEYBLOCK_EXE%, %UNINSTALL_EXE%
goto end

:error
echo Compilation failed!
pause

:end
pause
