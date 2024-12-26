#include "base.h"

static int cxscreen;
static int cyscreen;
static HHOOK mouse_hook;
static HHOOK keyboard_hook;
static bool recording = false;

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
    if (a->capacity == a->offset) {
        a->capacity <<= 1;
        a->data = realloc(a->data, a->capacity * sizeof(EventInput));
        if (a->data == NULL) {
            fputs("Failed to reallocate memory!\n", stderr);
            array_free(a);
            exit(1);
        }
    }
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
    if (a->capacity == a->offset) {
        a->capacity <<= 1;
        a->data = realloc(a->data, a->capacity * sizeof(EventInput));
        if (a->data == NULL) {
            fputs("Failed to reallocate memory!\n", stderr);
            array_free(a);
            exit(1);
        }
    }
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
    if (a->capacity == a->offset) {
        a->capacity <<= 1;
        a->data = realloc(a->data, a->capacity * sizeof(EventInput));
        if (a->data == NULL) {
            fputs("Failed to reallocate memory!\n", stderr);
            array_free(a);
            exit(1);
        }
    }
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

        if (wParam == WM_KEYUP) {
            keyboard_keys(&arr, key, KEYBOARD_UP);
        } else if (wParam == WM_KEYDOWN) {
            keyboard_keys(&arr, key, KEYBOARD_DOWN);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void replay(void) {
    LARGE_INTEGER start, difference, end, delta, frequency;
    QueryPerformanceFrequency(&frequency);

    for (u64 i = 1; i < arr.offset; ++i) {
        QueryPerformanceCounter(&start);
        SendInput(1, &arr.data[i].input, sizeof(INPUT));
        delta.QuadPart = (arr.data[i].time.QuadPart - arr.data[i - 1].time.QuadPart) * frequency.QuadPart / 1000000;
        
        do {
            Sleep(0);
            QueryPerformanceCounter(&end);
            difference.QuadPart = (end.QuadPart - start.QuadPart) * frequency.QuadPart / 1000000;
        } while (difference.QuadPart < delta.QuadPart);
    }
}

int main(void) {
    array_init(&arr);

    cxscreen = GetSystemMetrics(SM_CXSCREEN);
    cyscreen = GetSystemMetrics(SM_CYSCREEN);

    RegisterHotKey(NULL, 1, MOD_ALT, '1');
    RegisterHotKey(NULL, 2, MOD_ALT, '2');

    puts("Click ALT + 1 to record, and ALT + 2 to replay!");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY) {
            if (msg.wParam == 1) {
                if (!(recording)) {
                    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_callback, NULL, 0);
                    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);
                    recording = true;
                    
                    puts("Started recording. Click ALT + 1 again to stop!");
                } else {
                    UnhookWindowsHookEx(mouse_hook);
                    UnhookWindowsHookEx(keyboard_hook);
                    recording = false;

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
                    
                    puts("Recording stopped and saved!");
                }
            } else if (msg.wParam == 2) {
                arr.offset = 0;
                FILE *f = fopen("data.txt", "rb");
                if (f == NULL) {
                    fputs("Failed to open file for reading!\n", stderr);
                    goto cleanup;
                    return -1;
                }

                fread(&arr.offset, sizeof(u64), 1, f);
                while (arr.offset >= arr.capacity) {
                    arr.capacity <<= 1;
                    arr.data = realloc(arr.data, arr.capacity * sizeof(EventInput));
                    if (arr.data == NULL) {
                        fputs("Failed to reallocate memory!\n", stderr);
                        goto cleanup;
                        fclose(f);
                        return -1;
                    }
                }

                fread(arr.data, sizeof(EventInput), arr.offset, f);
                while (arr.offset >= arr.capacity) {
                    arr.capacity <<= 1;
                    arr.data = realloc(arr.data, arr.capacity * sizeof(EventInput));
                    if (arr.data == NULL) {
                        fputs("Failed to reallocate memory!\n", stderr);
                        goto cleanup;
                        fclose(f);
                        return -1;
                    }
                }

                if (ferror(f)) {
                    fputs("Failed to read data from the data.txt file!\n", stderr);
                    goto cleanup;
                    fclose(f);
                    return -1;
                }

                fclose(f);
                
                replay();
                puts("Replay finished!");
                break;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    cleanup:
        UnregisterHotKey(NULL, 1);
        UnregisterHotKey(NULL, 2);
        array_free(&arr);
    return 0;
}