#ifndef BASE_H
#define BASE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdint.h>

typedef int16_t s16;
typedef int32_t s32;

typedef uint16_t u16;
typedef uint64_t u64;

typedef float f32;

enum {
    MOUSE_MOVE,
	MOUSE_WHEEL,
    MOUSE_LEFT_UP,
    MOUSE_LEFT_DOWN,
    MOUSE_RIGHT_UP,
    MOUSE_RIGHT_DOWN,
    KEYBOARD_UP,
    KEYBOARD_DOWN,
};

#endif
