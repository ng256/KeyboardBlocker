/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Description: Main source file for Keyboard Blocker application.
 *              Implements window procedure, low-level keyboard hook,
 *              tray icon management, balloon notifications, and
 *              inter-process communication via mutex and event.
 *              Includes keyword detection ("unblock") to unblock,
 *              and tray menu commands to enable/disable blocking.
 */

#include "keyblock.h"
#include "resource.h"   // for icon IDs (IDI_ICON16, IDI_ICON32)

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

//-------------------------------------------------------------------
// Constants for keyword detection
//-------------------------------------------------------------------
#define KEYWORD     "unblock"
#define KEYWORD_LEN 7

//-------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------
HINSTANCE       g_hInst;
HANDLE          g_hMutex = NULL;
HANDLE          g_hStopEvent = NULL;
HANDLE          g_hWatchThread = NULL;
HHOOK           g_hHook = NULL;
HWND            g_hwnd = NULL;
NOTIFYICONDATA  g_nid = {0};

BOOL            g_bBlocking = TRUE;            // initially blocked
CRITICAL_SECTION g_cs;                         // protects g_typed and g_typedLen
char            g_typed[KEYWORD_LEN+1] = {0};  // last typed letters (linear buffer)
int             g_typedLen = 0;                // number of letters in buffer

//-------------------------------------------------------------------
// Helper: convert character to lowercase
//-------------------------------------------------------------------
static char to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 32;
    return c;
}

//-------------------------------------------------------------------
// Unblock keyboard (disable blocking) and show balloon
//-------------------------------------------------------------------
static void UnblockKeyboard(HWND hwnd)
{
    EnterCriticalSection(&g_cs);
    g_bBlocking = FALSE;
    // Clear typed buffer to avoid stale data
    g_typedLen = 0;
    g_typed[0] = '\0';
    LeaveCriticalSection(&g_cs);

    ShowBalloonUnblocked(hwnd);
}

//-------------------------------------------------------------------
// Block keyboard (enable blocking) and show balloon
//-------------------------------------------------------------------
static void BlockKeyboard(HWND hwnd)
{
    EnterCriticalSection(&g_cs);
    g_bBlocking = TRUE;
    // Clear typed buffer to start fresh
    g_typedLen = 0;
    g_typed[0] = '\0';
    LeaveCriticalSection(&g_cs);

    ShowBalloonBlocked(hwnd);
}

//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    // Mutex for single instance
    g_hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    if (g_hMutex == NULL)
        return 1;

    // Second instance signals the first to exit
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

    // First instance: create stop event
    g_hStopEvent = CreateEventA(NULL, TRUE, FALSE, EVENT_NAME);
    if (!g_hStopEvent)
    {
        CloseHandle(g_hMutex);
        return 1;
    }

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
        CloseHandle(g_hStopEvent);
        CloseHandle(g_hMutex);
        return 1;
    }

    // Create hidden window
    g_hwnd = CreateWindowExA(0, "KeyboardBlockerClass", "KeyboardBlocker",
                             0, 0, 0, 0, 0, NULL, NULL, g_hInst, NULL);
    if (!g_hwnd)
    {
        DeleteCriticalSection(&g_cs);
        CloseHandle(g_hStopEvent);
        CloseHandle(g_hMutex);
        return 1;
    }

    // Start watch thread (waits for stop event)
    g_hWatchThread = CreateThread(NULL, 0, WatchThreadProc, NULL, 0, NULL);
    if (!g_hWatchThread)
    {
        DestroyWindow(g_hwnd);
        DeleteCriticalSection(&g_cs);
        CloseHandle(g_hStopEvent);
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

//-------------------------------------------------------------------
// Window procedure
//-------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            AddTrayIcon(hwnd);
            ShowBalloonBlocked(hwnd);          // notify that keyboard is blocked
            // Set low-level keyboard hook
            g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                                        GetModuleHandleW(NULL), 0);
            if (!g_hHook)
                PostQuitMessage(1);
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP)
                ShowTrayMenu(hwnd);          // Right click
            else if (lParam == WM_LBUTTONUP)
            {
                if (g_bBlocking)
                    ShowBalloonBlocked(hwnd);
                else
                    ShowBalloonUnblocked(hwnd);
            }
            return 0;

        case WM_STOP_BLOCKING:
            // Signal from watch thread – exit the application
            ShowBalloonUnblocked(hwnd);
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            RemoveTrayIcon();
            if (g_hHook)
                UnhookWindowsHookEx(g_hHook);
            if (g_hWatchThread)
            {
                WaitForSingleObject(g_hWatchThread, 1000);
                CloseHandle(g_hWatchThread);
            }
            if (g_hStopEvent)
                CloseHandle(g_hStopEvent);
            if (g_hMutex)
                CloseHandle(g_hMutex);
            PostQuitMessage(0);
            return 0;

        case WM_QUERYENDSESSION:
            ShowBalloonUnblocked(hwnd);
            DestroyWindow(hwnd);
            return TRUE;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

//-------------------------------------------------------------------
// Keyboard hook – blocks or passes keys based on g_bBlocking,
// and detects the keyword "unblock" only when blocking is active.
//-------------------------------------------------------------------
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        // If blocking is active, we need to process keys for keyword detection
        if (g_bBlocking)
        {
            KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*)lParam;
            // Get the character for this virtual key (ignores dead keys)
            WORD ch = MapVirtualKeyA(p->vkCode, MAPVK_VK_TO_CHAR);
            if (ch && (ch & 0x8000) == 0)   // not a dead key
            {
                char c = (char)ch;
                c = to_lower(c);
                if (c >= 'a' && c <= 'z')   // only letters matter for keyword
                {
                    EnterCriticalSection(&g_cs);
                    // Update the linear buffer of last KEYWORD_LEN letters
                    if (g_typedLen < KEYWORD_LEN)
                    {
                        g_typed[g_typedLen] = c;
                        g_typedLen++;
                        g_typed[g_typedLen] = '\0';
                    }
                    else
                    {
                        // shift left and append
                        for (int i = 0; i < KEYWORD_LEN-1; i++)
                            g_typed[i] = g_typed[i+1];
                        g_typed[KEYWORD_LEN-1] = c;
                    }
                    // Check if buffer exactly matches the keyword
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
                        // Keyword detected – unblock the keyboard
                        LeaveCriticalSection(&g_cs);
                        UnblockKeyboard(g_hwnd);
                        return 1;   // block this key (the 'u' that completed the keyword)
                    }
                    LeaveCriticalSection(&g_cs);
                }
            }
        }
        // If blocking is not active, we don't process letters at all
    }

    // Decide whether to block or pass the key
    if (g_bBlocking)
        return 1;   // block all keys (including those already handled for detection)
    else
        return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//-------------------------------------------------------------------
// Watch thread: waits for stop event from second instance
//-------------------------------------------------------------------
DWORD WINAPI WatchThreadProc(LPVOID lpParam)
{
    WaitForSingleObject(g_hStopEvent, INFINITE);
    PostMessage(g_hwnd, WM_STOP_BLOCKING, 0, 0);
    return 0;
}

//-------------------------------------------------------------------
// Add tray icon (using resource IDI_ICON16) with white background removal
//-------------------------------------------------------------------
void AddTrayIcon(HWND hwnd)
{
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;

    // Load original icon from resources
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

//-------------------------------------------------------------------
// Remove tray icon
//-------------------------------------------------------------------
void RemoveTrayIcon()
{
    g_nid.uFlags = 0;
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

//-------------------------------------------------------------------
// Show balloon information
//-------------------------------------------------------------------
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

//-------------------------------------------------------------------
// Show block notification
//-------------------------------------------------------------------
void ShowBalloonBlocked(HWND hwnd)
{
    ShowBalloonMessage(hwnd, "Keyboard Blocker",
                       "Keyboard is blocked.\n"
                       "Run the app again, right-click the icon and select Exit, or type 'unblock' to unblock.",
                       NIIF_WARNING);
}

//-------------------------------------------------------------------
// Show unblock notification
//-------------------------------------------------------------------
void ShowBalloonUnblocked(HWND hwnd)
{
    ShowBalloonMessage(hwnd, "Keyboard Blocker",
                       "Keyboard is now unblocked.",
                       NIIF_INFO);
}

//-------------------------------------------------------------------
// Context menu on right click
//-------------------------------------------------------------------
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
