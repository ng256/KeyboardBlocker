/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project, released under MIT license.
 *
 * Description: Main source file for Keyboard Blocker application.
 *              Implements window procedure, low-level keyboard hook,
 *              tray icon management, balloon notifications, and
 *              inter-process communication via mutex and auto-reset event.
 *              Includes keyword detection ("unblock") to unblock,
 *              tray menu commands to enable/disable blocking,
 *              and registry persistence for the blocking state.
 */

#include "keyblock.h"
#include "resource.h"   // for icon IDs (IDI_ICON16, IDI_ICON32)

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

//=============================================================================
// Global variables
//=============================================================================

HINSTANCE       g_hInst;                // Application instance handle
HANDLE          g_hMutex = NULL;        // Mutex to ensure single instance
HANDLE          g_hEvent = NULL;        // Auto-reset event for status requests
HANDLE          g_hWatchThread = NULL;  // Thread that waits for event
HHOOK           g_hHook = NULL;         // Low-level keyboard hook handle
HWND            g_hwnd = NULL;          // Hidden window handle
NOTIFYICONDATA  g_nid = {0};            // Tray icon notification data

BOOL            g_bBlocking = TRUE;     // Current blocking state (TRUE = blocked)
CRITICAL_SECTION g_cs;                  // Protects g_typed and g_typedLen
char            g_typed[KEYWORD_LEN+1] = {0}; // Last KEYWORD_LEN letters typed (lowercase)
int             g_typedLen = 0;         // Number of valid letters in g_typed

//=============================================================================
// Registry persistence
//=============================================================================

// Save current blocking state to HKCU\Software\Keyblock\State (DWORD)
void SaveBlockingStateToRegistry(void)
{
    HKEY hKey;
    DWORD dwDisposition;
    LONG lResult = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Keyblock", 0, NULL,
                                   REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
    if (lResult == ERROR_SUCCESS)
    {
        DWORD value = TO_DWORD(g_bBlocking);
        RegSetValueExA(hKey, "State", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
}

// Load blocking state from registry; default to blocked if missing/invalid
void LoadBlockingStateFromRegistry(void)
{
    HKEY hKey;
    LONG lResult = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Keyblock", 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS)
    {
        DWORD value = 0;
        DWORD type = 0;
        DWORD size = sizeof(value);
        lResult = RegQueryValueExA(hKey, "State", NULL, &type, (BYTE*)&value, &size);
		// If registry key missing or non-zero, g_bBlocking remains TRUE (default)
        if (lResult == ERROR_SUCCESS && type == REG_DWORD)
        {
            g_bBlocking = TO_BOOL(value);
        }
        RegCloseKey(hKey);
    }
}

//=============================================================================
// Block/Unblock control (with registry sync and balloon)
//=============================================================================

void UnblockKeyboard(HWND hwnd)
{
    EnterCriticalSection(&g_cs);
    g_bBlocking = FALSE;
    g_typedLen = 0;          // clear typed buffer
    g_typed[0] = '\0';
    LeaveCriticalSection(&g_cs);
    SaveBlockingStateToRegistry();
    ShowBalloonUnblocked(hwnd);
}

void BlockKeyboard(HWND hwnd)
{
    EnterCriticalSection(&g_cs);
    g_bBlocking = TRUE;
    g_typedLen = 0;          // clear typed buffer
    g_typed[0] = '\0';
    LeaveCriticalSection(&g_cs);
    SaveBlockingStateToRegistry();
    ShowBalloonBlocked(hwnd);
}

//=============================================================================
// WinMain – entry point
//=============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    // Single instance control via mutex
    g_hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    if (g_hMutex == NULL)
        return 1;

    // Second instance: signal the first to show status and quit
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, EVENT_NAME);
        if (hEvent)
        {
            SetEvent(hEvent);
            CloseHandle(hEvent);
        }
        CloseHandle(g_hMutex);
        return 0;
    }

    // First instance: create auto-reset event
    g_hEvent = CreateEventA(NULL, FALSE, FALSE, EVENT_NAME);
    if (!g_hEvent)
    {
        CloseHandle(g_hMutex);
        return 1;
    }

    // Load saved blocking state from registry
    LoadBlockingStateFromRegistry();

    // Initialize critical section for keyword buffer
    InitializeCriticalSection(&g_cs);

    // Register window class
    WNDCLASSEXA wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = g_hInst;
    wc.lpszClassName = "KeyboardBlockerClass";
    wc.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON32));
    wc.hIconSm       = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON16));
    if (!RegisterClassExA(&wc))
    {
        DeleteCriticalSection(&g_cs);
        CloseHandle(g_hEvent);
        CloseHandle(g_hMutex);
        return 1;
    }

    // Create hidden window (used for message handling and tray icon)
    g_hwnd = CreateWindowExA(0, "KeyboardBlockerClass", "KeyboardBlocker",
                             0, 0, 0, 0, 0, NULL, NULL, g_hInst, NULL);
    if (!g_hwnd)
    {
        DeleteCriticalSection(&g_cs);
        CloseHandle(g_hEvent);
        CloseHandle(g_hMutex);
        return 1;
    }

    // Start thread that waits for the event (from second instances)
    g_hWatchThread = CreateThread(NULL, 0, WatchThreadProc, NULL, 0, NULL);
    if (!g_hWatchThread)
    {
        DestroyWindow(g_hwnd);
        DeleteCriticalSection(&g_cs);
        CloseHandle(g_hEvent);
        CloseHandle(g_hMutex);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    DeleteCriticalSection(&g_cs);
    return 0;
}

//=============================================================================
// Window procedure
//=============================================================================

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            AddTrayIcon(hwnd);
            // Show balloon according to current blocking state
            if (g_bBlocking)
                ShowBalloonBlocked(hwnd);
            else
                ShowBalloonUnblocked(hwnd);
            // Install low-level keyboard hook
            g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                                        GetModuleHandleW(NULL), 0);
            if (!g_hHook)
                PostQuitMessage(1);
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP)
                ShowTrayMenu(hwnd);          // Right click: show context menu
            else if (lParam == WM_LBUTTONUP) // Left click: show current state balloon
            {
                if (g_bBlocking)
                    ShowBalloonBlocked(hwnd);
                else
                    ShowBalloonUnblocked(hwnd);
            }
            return 0;

        case WM_SHOW_STATUS:
            // Show current status balloon (triggered by second instance)
            if (g_bBlocking)
                ShowBalloonBlocked(hwnd);
            else
                ShowBalloonUnblocked(hwnd);
            return 0;

        case WM_DESTROY:
            SaveBlockingStateToRegistry();   // Persist final state
            RemoveTrayIcon();
            if (g_hHook)
                UnhookWindowsHookEx(g_hHook);
            if (g_hWatchThread)
            {
                WaitForSingleObject(g_hWatchThread, 1000);
                CloseHandle(g_hWatchThread);
            }
            if (g_hEvent)
                CloseHandle(g_hEvent);
            if (g_hMutex)
                CloseHandle(g_hMutex);
            PostQuitMessage(0);
            return 0;

        case WM_QUERYENDSESSION:
            // System shutdown: save state and clean up
            SaveBlockingStateToRegistry();
            ShowBalloonUnblocked(hwnd);
            DestroyWindow(hwnd);
            return TRUE;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

//=============================================================================
// Low-level keyboard hook procedure
//=============================================================================

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        // Only process letters when blocking is active (to detect "unblock")
        if (g_bBlocking)
        {
            KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*)lParam;
            WORD ch = MapVirtualKeyA(p->vkCode, MAPVK_VK_TO_CHAR);
            if (ch && (ch & 0x8000) == 0)   // not a dead key
            {
                char c = (char)ch;
                c = TO_LOWER(c);            // macro inlined
                if (c >= 'a' && c <= 'z')   // only letters matter
                {
                    EnterCriticalSection(&g_cs);
                    // Maintain circular buffer of last KEYWORD_LEN letters
                    if (g_typedLen < KEYWORD_LEN)
                    {
                        g_typed[g_typedLen] = c;
                        g_typedLen++;
                        g_typed[g_typedLen] = '\0';
                    }
                    else
                    {
                        for (int i = 0; i < KEYWORD_LEN-1; i++)
                            g_typed[i] = g_typed[i+1];
                        g_typed[KEYWORD_LEN-1] = c;
                    }

                    // Check for keyword match
                    int match = 1;
                    for (int i = 0; i < KEYWORD_LEN; i++)
                    {
                        if (g_typed[i] != KEYWORD[i])
                        {
                            match = 0;
                            break;
                        }
                    }
                    if (match && g_typedLen == KEYWORD_LEN)
                    {
                        LeaveCriticalSection(&g_cs);
                        UnblockKeyboard(g_hwnd);   // toggle off blocking
                        return 1;   // block the final 'u' key that completed the word
                    }
                    LeaveCriticalSection(&g_cs);
                }
            }
        }
    }

    // Pass or block based on current blocking state
    if (g_bBlocking)
        return 1;   // block all keys
    else
        return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//=============================================================================
// Watch thread: waits for event (signaled by second instances)
//=============================================================================

DWORD WINAPI WatchThreadProc(LPVOID lpParam)
{
    while (1)
    {
        WaitForSingleObject(g_hEvent, INFINITE);
        PostMessage(g_hwnd, WM_SHOW_STATUS, 0, 0);
    }
    return 0;
}

//=============================================================================
// Tray icon management
//=============================================================================

void AddTrayIcon(HWND hwnd)
{
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;

    // Load icon from resources
    HICON hOriginalIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON16),
                                           IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (!hOriginalIcon)
        hOriginalIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION),
                                         IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    g_nid.hIcon = hOriginalIcon;
    lstrcpyA(g_nid.szTip, "Keyboard Blocker");
    Shell_NotifyIconA(NIM_ADD, &g_nid);
    DestroyIcon(hOriginalIcon);
}

void RemoveTrayIcon()
{
    g_nid.uFlags = 0;
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

//=============================================================================
// Balloon notifications
//=============================================================================

void ShowBalloonMessage(HWND hwnd, const char* title, const char* text, DWORD infoFlags)
{
    NOTIFYICONDATAA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_INFO;

    lstrcpyA(nid.szInfoTitle, title);
    lstrcpyA(nid.szInfo, text);
    nid.dwInfoFlags = infoFlags;
    nid.uTimeout = 10000;

    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void ShowBalloonBlocked(HWND hwnd)
{
    ShowBalloonMessage(hwnd, "Keyboard Blocker",
                       "Keyboard is blocked.\n"
                       "Type 'unblock' to unblock, or right-click the icon.",
                       NIIF_WARNING);
}

void ShowBalloonUnblocked(HWND hwnd)
{
    ShowBalloonMessage(hwnd, "Keyboard Blocker",
                       "Keyboard is now unblocked.",
                       NIIF_INFO);
}

//=============================================================================
// Tray icon context menu
//=============================================================================

void ShowTrayMenu(HWND hwnd)
{
    HMENU hMenu = CreatePopupMenu();

    // Add Block / Unblock items (mutually exclusive)
    if (g_bBlocking)
    {
        AppendMenuA(hMenu, MF_STRING | MF_DISABLED, 2, "Block");
        AppendMenuA(hMenu, MF_STRING, 3, "Unblock");
    }
    else
    {
        AppendMenuA(hMenu, MF_STRING, 2, "Block");
        AppendMenuA(hMenu, MF_STRING | MF_DISABLED, 3, "Unblock");
    }

    AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hMenu, MF_STRING, 1, "Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);

    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                              pt.x, pt.y, 0, hwnd, NULL);

    if (cmd == 1)       // Exit
    {
        DestroyWindow(hwnd);
    }
    else if (cmd == 2)  // Block
    {
        BlockKeyboard(hwnd);
    }
    else if (cmd == 3)  // Unblock
    {
        UnblockKeyboard(hwnd);
    }

    DestroyMenu(hMenu);
}
