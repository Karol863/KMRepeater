#ifndef BASE_H
#define BASE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdint.h>

typedef uint64_t u64;

enum {
    CAPACITY = 4096,
    MOUSE_MOVE,
    MOUSE_LEFT_UP,
    MOUSE_LEFT_DOWN,
    MOUSE_RIGHT_UP,
    MOUSE_RIGHT_DOWN,
    KEYBOARD_UP,
    KEYBOARD_DOWN,
};

#endif