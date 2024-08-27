#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

#define initial_capacity 1024

static int cxscreen;
static int cyscreen;
static HHOOK mouse_hook;
static HHOOK keyboard_hook;
static bool recording = false;

enum {
    MOUSE_MOVE,
    MOUSE_LEFT_UP,
    MOUSE_LEFT_DOWN,
    MOUSE_RIGHT_UP,
    MOUSE_RIGHT_DOWN,
    KEYBOARD_UP,
    KEYBOARD_DOWN,
};

typedef struct {
    INPUT input;
    LARGE_INTEGER time;
} EventInput;

typedef struct {
    EventInput *data;
    size_t size;
    size_t capacity;
} a;

static void init_array(a *arr) {
    arr->data = (EventInput *)malloc(initial_capacity * sizeof(EventInput));
    arr->size = 0;
    arr->capacity = initial_capacity;
}

static void resize_array_mouse_move(a *arr, POINT coordinates, WPARAM wParam) {
    if (arr->capacity == arr->size) {
        arr->capacity *= 2;
        arr->data = (EventInput *)realloc(arr->data, arr->capacity * sizeof(EventInput));
    }
    arr->data[arr->size].input.type = INPUT_MOUSE;
    arr->data[arr->size].input.mi.dx = coordinates.x * (65535 / cxscreen);
    arr->data[arr->size].input.mi.dy = coordinates.y * (65535 / cyscreen);

    switch(wParam) {
        case MOUSE_MOVE:
            arr->data[arr->size].input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
            break;
        default:
            break;
    }
    QueryPerformanceCounter(&arr->data[arr->size].time);
    arr->size++;
}

static void resize_array_mouse(a *arr, WPARAM wParam) {
    if (arr->capacity == arr->size) {
        arr->capacity *= 2;
        arr->data = (EventInput *)realloc(arr->data, arr->capacity * sizeof(EventInput));
    }
    arr->data[arr->size].input.type = INPUT_MOUSE;
    
    switch(wParam) {
        case MOUSE_LEFT_UP:
            arr->data[arr->size].input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            break;
        case MOUSE_RIGHT_UP:
            arr->data[arr->size].input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            break;
        case MOUSE_LEFT_DOWN:
            arr->data[arr->size].input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            break;
        case MOUSE_RIGHT_DOWN:
            arr->data[arr->size].input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            break;
        default:
            break;
    }
    QueryPerformanceCounter(&arr->data[arr->size].time);
    arr->size++;
}

static void resize_array_keyboard(a *arr, DWORD key, WPARAM wParam) {
    if (arr->capacity == arr->size) {
        arr->capacity *= 2;
        arr->data = (EventInput *)realloc(arr->data, arr->capacity * sizeof(EventInput));
    }
    arr->data[arr->size].input.type = INPUT_KEYBOARD;
    arr->data[arr->size].input.ki.wVk = key;

    switch(wParam) {
        case KEYBOARD_UP:
            arr->data[arr->size].input.ki.dwFlags = KEYEVENTF_KEYUP;
            break;
        case KEYBOARD_DOWN:
            arr->data[arr->size].input.ki.dwFlags = 0;
            break;
        default:
            break;
    }
    QueryPerformanceCounter(&arr->data[arr->size].time);
    arr->size++;
}

static void free_array(a *arr) {
    free(arr->data);
    arr->size = 0;
    arr->capacity = 0;
}

a array;

static LRESULT CALLBACK mouse(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *) lParam;
        POINT coordinates = p->pt;

        switch(wParam) {
            case WM_MOUSEMOVE:
                resize_array_mouse_move(&array, coordinates, MOUSE_MOVE);
                break;
            case WM_LBUTTONUP:
                resize_array_mouse(&array, MOUSE_LEFT_UP);
                break;
            case WM_RBUTTONUP:
                resize_array_mouse(&array, MOUSE_RIGHT_UP);
                break;
            case WM_LBUTTONDOWN:
                resize_array_mouse(&array, MOUSE_LEFT_DOWN);
                break;
            case WM_RBUTTONDOWN:
                resize_array_mouse(&array, MOUSE_RIGHT_DOWN);
                break;
            default:
                break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static LRESULT CALLBACK keyboard(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *) lParam;
        DWORD key = p->vkCode;

        switch(wParam) {
            case WM_KEYUP:
                resize_array_keyboard(&array, key, KEYBOARD_UP);
                break;
            case WM_KEYDOWN:
                resize_array_keyboard(&array, key, KEYBOARD_DOWN);
                break;
            default:
                break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void replay(void) {
    LARGE_INTEGER start, difference, end, delta, frequency;
    QueryPerformanceFrequency(&frequency);

    for (size_t i = 1; i < array.size; i++) {
        QueryPerformanceCounter(&start);
        SendInput(1, &array.data[i].input, sizeof(INPUT));
        delta.QuadPart = (array.data[i].time.QuadPart - array.data[i - 1].time.QuadPart) * frequency.QuadPart / 1000000;
        
        do {
            Sleep(0);
            QueryPerformanceCounter(&end);
            difference.QuadPart = end.QuadPart - start.QuadPart;
        } while (difference.QuadPart < delta.QuadPart);
    }
}

int main(void) {
    init_array(&array);
    cxscreen = GetSystemMetrics(SM_CXSCREEN);
    cyscreen = GetSystemMetrics(SM_CYSCREEN);

    RegisterHotKey(NULL, 1, MOD_ALT, '1');
    RegisterHotKey(NULL, 2, MOD_ALT, '2');

    puts("Click ALT + 1 to record, and ALT + 2 to replay!");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            if (msg.wParam == 1) {
                if (!(recording)) {
                    array.size = 0;
                    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse, NULL, 0);
                    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard, NULL, 0);
                    recording = true;
                    
                    puts("Started recording. Click ALT + 1 once again to stop!");
                } else {
                    UnhookWindowsHookEx(mouse_hook);
                    UnhookWindowsHookEx(keyboard_hook);
                    recording = false;

                    FILE *fp = fopen("data.txt", "wb");
                    if (fp == NULL) {
                        fputs("Failed to open file for writing!\n", stderr);
                        UnregisterHotKey(NULL, 1);
                        UnregisterHotKey(NULL, 2);
                        free_array(&array);
                        return -1;
                    }
                    
                    fwrite(&array.size, sizeof(size_t), 1, fp);
                    fwrite(array.data, sizeof(EventInput), array.size, fp);
                    fclose(fp);
                    
                    puts("Recording stopped and saved!");
                }
            } else if (msg.wParam == 2) {
                array.size = 0;
                FILE *fp = fopen("data.txt", "rb");
                if (fp == NULL) {
                    fputs("Failed to open file for reading!\n", stderr);
                    UnregisterHotKey(NULL, 1);
                    UnregisterHotKey(NULL, 2);
                    free_array(&array);
                    return -1;
                }
                fread(&array.size, sizeof(size_t), 1, fp);
                
                if (array.size > array.capacity) {
                    array.data = (EventInput *)realloc(array.data, array.size * sizeof(EventInput));
                    array.capacity = array.size;
                }
                
                fread(array.data, sizeof(EventInput), array.size, fp);
                fclose(fp);
                
                replay();
                puts("Replay finished!");
                break;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterHotKey(NULL, 1);
    UnregisterHotKey(NULL, 2);
    free_array(&array);
    return 0;
}
