/*
 * Keyboard Blocker - Windows application to block keyboard input
 * Copyright (c) 2025 Pavel Bashkardin
 *
 * This file is part of Keyboard Blocker project, released under MIT license.
 *
 * Description: Minimal CRT replacement implementations.
 */

#include "nocrt.h"

void* __cdecl memmove(void* dest, const void* src, size_t count)
{
    RtlMoveMemory(dest, src, count);
    return dest;
}

void* __cdecl memset(void* dest, int c, size_t count)
{
    unsigned char* d = (unsigned char*)dest;
    while (count--)
        *d++ = (unsigned char)c;
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count)
{
    // RtlMoveMemory handles overlapping correctly, so it's safe for memcpy too.
    RtlMoveMemory(dest, src, count);
    return dest;
}