#include "base.h"

static int cxscreen;
static int cyscreen;
static HHOOK mouse_hook;
static HHOOK keyboard_hook;

typedef struct {
    INPUT input;
    LARGE_INTEGER time;
} EventInput;

typedef struct {
    EventInput *data;
    u64 offset;
    u64 capacity;
} Array;

static void array_init(Array *a) {
    a->data = malloc(CAPACITY * sizeof(EventInput));
    if (a->data == NULL) {
        fputs("Failed to allocate memory!\n", stderr);
        exit(1);
    }
    a->offset = 0;
    a->capacity = CAPACITY;
}

static void array_free(Array *a) {
    free(a->data);
    a->offset = 0;
    a->capacity = 0;
}

static void mouse_move(Array *a, POINT coordinates, WPARAM wParam) {
    a->data[a->offset].input.type = INPUT_MOUSE;
    a->data[a->offset].input.mi.dx = coordinates.x * (65535 / cxscreen);
    a->data[a->offset].input.mi.dy = coordinates.y * (65535 / cyscreen);

    if (wParam == MOUSE_MOVE) {
        a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    }

    QueryPerformanceCounter(&a->data[a->offset].time);
    a->offset++;
}

static void mouse_buttons(Array *a, WPARAM wParam) {
    a->data[a->offset].input.type = INPUT_MOUSE;
    
    if (wParam == MOUSE_LEFT_UP) {
        a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    } else if (wParam == MOUSE_RIGHT_UP) {
        a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    } else if (wParam == MOUSE_LEFT_DOWN) {
        a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    } else if (wParam == MOUSE_RIGHT_DOWN) {
        a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    }

    QueryPerformanceCounter(&a->data[a->offset].time);
    a->offset++;
}

static void keyboard_keys(Array *a, DWORD key, WPARAM wParam) {
    a->data[a->offset].input.type = INPUT_KEYBOARD;
    a->data[a->offset].input.ki.wVk = key;

    if (wParam == KEYBOARD_UP) {
        a->data[a->offset].input.ki.dwFlags = KEYEVENTF_KEYUP;
    } else if (wParam == KEYBOARD_DOWN) {
        a->data[a->offset].input.ki.dwFlags = 0;
    }

    QueryPerformanceCounter(&a->data[a->offset].time);
    a->offset++;
}

Array arr;

static LRESULT CALLBACK mouse_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
        POINT coordinates = p->pt;
        
        if (arr.capacity == arr.offset) {
            arr.capacity *= 2;
            arr.data = realloc(arr.data, arr.capacity * sizeof(EventInput));
            if (arr.data == NULL) {
                fputs("Failed to reallocate memory!\n", stderr);
                array_free(&arr);
                exit(1);
            }
        }

        if (wParam == WM_MOUSEMOVE) {
            mouse_move(&arr, coordinates, MOUSE_MOVE);
        } else if (wParam == WM_LBUTTONUP) {
            mouse_buttons(&arr, MOUSE_LEFT_UP);
        } else if (wParam == WM_RBUTTONUP) {
            mouse_buttons(&arr, MOUSE_RIGHT_UP);
        } else if (wParam == WM_LBUTTONDOWN) {
            mouse_buttons(&arr, MOUSE_LEFT_DOWN);
        } else if (wParam == WM_RBUTTONDOWN) {
            mouse_buttons(&arr, MOUSE_RIGHT_DOWN);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static LRESULT CALLBACK keyboard_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        DWORD key = p->vkCode;

        if (arr.capacity == arr.offset) {
            arr.capacity *= 2;
            arr.data = realloc(arr.data, arr.capacity * sizeof(EventInput));
            if (arr.data == NULL) {
                fputs("Failed to reallocate memory!\n", stderr);
                array_free(&arr);
                exit(1);
            }
        }

        if (wParam == WM_KEYUP) {
            keyboard_keys(&arr, key, KEYBOARD_UP);
        } else if (wParam == WM_KEYDOWN) {
            keyboard_keys(&arr, key, KEYBOARD_DOWN);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void replay(void) {
    LARGE_INTEGER start, difference, end, delta;

    for (u64 i = 1; i < arr.offset; ++i) {
        QueryPerformanceCounter(&start);
        SendInput(1, &arr.data[i].input, sizeof(INPUT));
        delta.QuadPart = arr.data[i].time.QuadPart - arr.data[i - 1].time.QuadPart;
        
        do {
            Sleep(0);
            QueryPerformanceCounter(&end);
            difference.QuadPart = end.QuadPart - start.QuadPart;
        } while (difference.QuadPart < delta.QuadPart);
    }
}

int main(void) {
    array_init(&arr);
    char number[4096];

    cxscreen = GetSystemMetrics(SM_CXSCREEN);
    cyscreen = GetSystemMetrics(SM_CYSCREEN);

    RegisterHotKey(NULL, 1, MOD_ALT, '1');
    RegisterHotKey(NULL, 2, MOD_ALT, '2');
    RegisterHotKey(NULL, 3, MOD_ALT, '3');
    RegisterHotKey(NULL, 4, MOD_ALT, '4');

    puts("Click ALT + 1 to record.");
    puts("Click ALT + 2 to save.");
    puts("Click ALT + 3 to replay from the current buffer.");
    puts("Click ALT + 4 to replay from the 'data.txt' file.");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_callback, NULL, 0);
            keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);

            puts("Recording...");
        } else if (msg.message == WM_HOTKEY && msg.wParam == 2) {
            UnhookWindowsHookEx(mouse_hook);
            UnhookWindowsHookEx(keyboard_hook);

            FILE *f = fopen("data.txt", "wb");
            if (f == NULL) {
                fputs("Failed to open file for writing!\n", stderr);
                goto cleanup;
                return -1;
            }
                    
            fwrite(&arr.offset, sizeof(u64), 1, f);
            fwrite(arr.data, sizeof(EventInput), arr.offset, f);
            if (ferror(f)) {
                fputs("Failed to write data into the data.txt file!\n", stderr);
                goto cleanup;
                fclose(f);
                return -1;
            }
            fclose(f);
                    
            puts("Saved!");
        } else if (msg.message == WM_HOTKEY && msg.wParam == 3) {
            UnhookWindowsHookEx(mouse_hook);
            UnhookWindowsHookEx(keyboard_hook);
            
            printf("How many times you'd like to replay? In case of an infinite replay, enter 0: ");
            if (fgets(number, sizeof(number), stdin) == NULL) {
                fputs("Failed to read user input!\n", stderr);
                goto cleanup;
                exit(1);
            }
            printf("Choosen number: %s\n", number);
            
            int times = atoi(number);
            if (times == 0) {
                for (;;) {
                    replay();
                }
            } else {
                for (int i = 0; i < times; ++i) {
                    replay();
                    printf("Passed replay: %d\n", i + 1);
                }
            }
            
            puts("Replay finished!");
            break;
        } else if (msg.message == WM_HOTKEY && msg.wParam == 4) {
            mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_callback, NULL, 0);
            keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);

            FILE *f = fopen("data.txt", "rb");
            if (f == NULL) {
                fputs("Failed to open file for reading!\n", stderr);
                goto cleanup;
                return -1;
            }
                    
            fread(&arr.offset, sizeof(u64), 1, f);
            
            arr.data = realloc(arr.data, arr.offset * sizeof(EventInput));
            if (arr.data == NULL) {
                fputs("Failed to allocate memory!\n", stderr);
                goto cleanup;
                fclose(f);
                return -1;
            }

            fread(arr.data, sizeof(EventInput), arr.offset, f);
            if (ferror(f)) {
                fputs("Failed to read data from the data.txt file!\n", stderr);
                goto cleanup;
                fclose(f);
                return -1;
            }
            fclose(f);
            
            UnhookWindowsHookEx(mouse_hook);
            UnhookWindowsHookEx(keyboard_hook);

            printf("How many times you'd like to replay? In case of an infinite replay, enter '0': ");
            if (fgets(number, sizeof(number), stdin) == NULL) {
                fputs("Failed to read user input!\n", stderr);
                goto cleanup;
                exit(1);
            }
            printf("Choosen number: %s\n", number);
            
            int times = atoi(number);
            if (times == 0) {
                for (;;) {
                    replay();
                }
            } else {
                for (int i = 0; i < times; ++i) {
                    replay();
                    printf("Replay passed: %d\n", i + 1);
                }
            }

            puts("Replay finished!");
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    array_free(&arr);
    return 0;

    cleanup:
        UnregisterHotKey(NULL, 1);
        UnregisterHotKey(NULL, 2);
        UnregisterHotKey(NULL, 3);
        UnregisterHotKey(NULL, 4);
        array_free(&arr);
}