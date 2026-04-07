/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project, released under MIT license.
 *
 * Description: Header file for Keyboard Blocker application. Declares constants,
 *              global variables, and function prototypes used for keyboard blocking,
 *              tray icon management, inter-process communication, and registry
 *              persistence. Also defines the keyword for unblocking.
 */

#ifndef KEYBOARDBLOCKER_H
#define KEYBOARDBLOCKER_H

#include <windows.h>
#include <shellapi.h>

//=============================================================================
// Constants
//=============================================================================

// Inter-process communication
#define MUTEX_NAME          "KeyboardBlockerMutex"   // Mutex for single instance
#define EVENT_NAME          "KeyboardBlockerEvent"   // Auto-reset event for status request

// Custom window messages
#define WM_TRAYICON         (WM_APP + 1)             // Tray icon notification
#define WM_SHOW_STATUS      (WM_APP + 2)             // Show current status balloon

// Keyword detection
#define KEYWORD             "unblock"                // Keyword to unblock keyboard
#define KEYWORD_LEN         7                        // Length of keyword

// Utility macros
#define TO_LOWER(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + 32 : (c)) // Convert character to lowercase
#define TO_BOOL(v)  ((v) != 0)                                  // Convert DWORD to BOOL (non-zero = TRUE)
#define TO_DWORD(v)  v ? 1 : 0                                  // Convert BOOL to DWORD

//=============================================================================
// Global variables (defined in keyblock.c)
//=============================================================================

extern HINSTANCE       g_hInst;          // Application instance handle
extern HANDLE          g_hMutex;         // Mutex to ensure single instance
extern HANDLE          g_hEvent;         // Auto-reset event for status requests
extern HANDLE          g_hWatchThread;   // Thread that waits for event
extern HHOOK           g_hHook;          // Low-level keyboard hook handle
extern HWND            g_hwnd;           // Hidden window handle
extern NOTIFYICONDATA  g_nid;            // Tray icon notification data

// Additional globals used by keyboard hook and blocking control
extern BOOL            g_bBlocking;      // Current blocking state (TRUE = blocked)
extern CRITICAL_SECTION g_cs;            // Critical section for keyword buffer
extern char            g_typed[];        // Last KEYWORD_LEN letters typed (lowercase)
extern int             g_typedLen;       // Number of valid letters in g_typed

//=============================================================================
// Function prototypes
//=============================================================================

// Main window and hook procedures
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI     WatchThreadProc(LPVOID lpParam);

// Tray icon management
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon(void);
void ShowTrayMenu(HWND hwnd);

// Balloon notifications
void ShowBalloonMessage(HWND hwnd, const char* title, const char* text, DWORD infoFlags);
void ShowBalloonBlocked(HWND hwnd);
void ShowBalloonUnblocked(HWND hwnd);

// Registry persistence
void SaveBlockingStateToRegistry(void);
void LoadBlockingStateFromRegistry(void);

// Block/Unblock control
void BlockKeyboard(HWND hwnd);
void UnblockKeyboard(HWND hwnd);

#endif // KEYBOARDBLOCKER_H
