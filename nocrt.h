/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project, released under MIT license.
 *
 * Description: Minimal CRT replacement functions to avoid msvcrt.dll dependency.
 */

#ifndef NOCRT_H
#define NOCRT_H

#include <windows.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* __cdecl memmove(void* dest, const void* src, size_t count);
void* __cdecl memset(void* dest, int c, size_t count);
void* __cdecl memcpy(void* dest, const void* src, size_t count);

// We may also need memcmp, strlen, etc. if used.
// Add them if compiler complains.

#ifdef __cplusplus
}
#endif

#endif // NOCRT_H