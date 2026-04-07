/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project, released under MIT license.
 *
 * Description: uninstall program source file.
 *              During uninstallation:
 *              - The application process will be terminated
 *              - All registry entries will be removed
 *              - Shortcuts will be deleted
 *              - All files, including the installation folder, will be removed.
 */

#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "Shlwapi.lib")

// Delete shortcuts
void RemoveShortcuts()
{
    char path[MAX_PATH];

    // Desktop shortcut
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
    {
        PathAppendA(path, "Keyboard Blocker.lnk");
        DeleteFileA(path);
    }

    // Start Menu shortcut
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL, 0, path)))
    {
        PathAppendA(path, "Keyboard Blocker.lnk");
        DeleteFileA(path);
    }
}

// Delete registry keys
void RemoveRegistry()
{
	// Remove app settings
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Keyblock");
	
	// Remove startup entry
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) 
	{
		RegDeleteValue(hKey, "Keyboard Blocker");
		RegCloseKey(hKey);
	}

    // Remove uninstall entry
    SHDeleteKeyA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Keyboard Blocker"
    );
}

// Recursively delete directory
BOOL DeleteDirectoryRecursive(LPCSTR dir)
{
    char searchPath[MAX_PATH];
    WIN32_FIND_DATAA ffd;
    HANDLE hFind;

    lstrcpyA(searchPath, dir);
    PathAppendA(searchPath, "*");

    hFind = FindFirstFileA(searchPath, &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        return FALSE;

    do
    {
        if (lstrcmpA(ffd.cFileName, ".") == 0 ||
            lstrcmpA(ffd.cFileName, "..") == 0)
            continue;

        char fullPath[MAX_PATH];
        lstrcpyA(fullPath, dir);
        PathAppendA(fullPath, ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            DeleteDirectoryRecursive(fullPath);
            RemoveDirectoryA(fullPath);
        }
        else
        {
            SetFileAttributesA(fullPath, FILE_ATTRIBUTE_NORMAL);
            DeleteFileA(fullPath);
        }

    } while (FindNextFileA(hFind, &ffd));

    FindClose(hFind);

    return TRUE;
}

// Kill process by name
void KillProcessByName(LPCSTR processName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(hSnap, &pe))
    {
        do
        {
            if (_stricmp(pe.szExeFile, processName) == 0)
            {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc)
                {
                    TerminateProcess(hProc, 0);
                    CloseHandle(hProc);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	// Confirm uninstallation
	int result = MessageBoxA(
		NULL,
		"Are you sure you want to uninstall Keyboard Blocker?",
		"Keyboard Blocker",
		MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2
	);

	if (result != IDOK)
	{
		return 0;
	}
	
    char exePath[MAX_PATH];
    char dirPath[MAX_PATH];

    // Get full path to current exe
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Get directory
    lstrcpyA(dirPath, exePath);
    PathRemoveFileSpecA(dirPath);

    // Kill keyblock.exe if running
    KillProcessByName("keyblock.exe");
	
	// Remove shortcuts
	RemoveShortcuts();

    // Remove registry
    RemoveRegistry();

    // Delete files in directory
    DeleteDirectoryRecursive(dirPath);

    // Self-delete via cmd
    char cmd[MAX_PATH * 2];
    wsprintfA(
        cmd,
        "cmd.exe /c ping 127.0.0.1 -n 2 > nul & rd /s /q \"%s\"",
        dirPath
    );

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
