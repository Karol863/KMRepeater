#include "array.c"

static s32 delta;

int cxscreen;
int cyscreen;
Array arr;
BOOL replaying = TRUE;

static void mouse_handler(Array *a, WPARAM wParam, POINT coordinates, DWORD wheel) {
	if (a->offset == a->capacity) {
		a->capacity *= 2;
		array_alloc(a, a->capacity);
	}

    a->data[a->offset].input.type = INPUT_MOUSE;
    a->data[a->offset].input.mi.dx = coordinates.x * (65535 / cxscreen);
    a->data[a->offset].input.mi.dy = coordinates.y * (65535 / cyscreen);
	a->data[a->offset].input.mi.mouseData = wheel;

	switch(wParam) {
		case MOUSE_MOVE:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
			break;
		case MOUSE_WHEEL:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_WHEEL | MOUSEEVENTF_HWHEEL;
			break;
		case MOUSE_LEFT_UP:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			break;
		case MOUSE_RIGHT_UP:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
			break;
		case MOUSE_LEFT_DOWN:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			break;
		case MOUSE_RIGHT_DOWN:
			a->data[a->offset].input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
			break;
		default:
			break;
	}

    QueryPerformanceCounter(&a->data[a->offset].time);
    a->offset++;
}

static void keyboard_handler(Array *a, WPARAM wParam, DWORD key) {
	if (a->offset == a->capacity) {
		a->capacity *= 2;
		array_alloc(a, a->capacity);
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

LRESULT CALLBACK mouse_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
		POINT empty = {0, 0};

		switch(wParam) {
			case WM_MOUSEMOVE:
				mouse_handler(&arr, MOUSE_MOVE, p->pt, 0);
				break;
			case WM_MOUSEWHEEL:
			case WM_MOUSEHWHEEL:
				delta = (s16)(p->mouseData >> 16);
				mouse_handler(&arr, MOUSE_WHEEL, empty, delta);
				break;
			case WM_LBUTTONUP:
				mouse_handler(&arr, MOUSE_LEFT_UP, empty, 0);
				break;
			case WM_RBUTTONUP:
				mouse_handler(&arr, MOUSE_RIGHT_UP, empty, 0);
				break;
			case WM_LBUTTONDOWN:
				mouse_handler(&arr, MOUSE_LEFT_DOWN, empty, 0);
				break;
			case WM_RBUTTONDOWN:
				mouse_handler(&arr, MOUSE_RIGHT_DOWN, empty, 0);
				break;
			default:
				break;
		}
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK keyboard_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        DWORD key = p->vkCode;

        if (wParam == WM_KEYUP) {
			keyboard_handler(&arr, KEYBOARD_UP, key);
        } else if (wParam == WM_KEYDOWN) {
			keyboard_handler(&arr, KEYBOARD_DOWN, key);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void replay(f32 speed) {
    LARGE_INTEGER start, difference, end, delta;

    for (u64 i = 1; i < arr.offset; ++i) {
		if (GetKeyState(VK_F4) & 0x8000) {
			replaying = FALSE;
		}

		delta.QuadPart = arr.data[i].time.QuadPart - arr.data[i - 1].time.QuadPart;
		delta.QuadPart = delta.QuadPart / speed;

        QueryPerformanceCounter(&start);
        SendInput(1, &arr.data[i].input, sizeof(INPUT));

        do {
			YieldProcessor();
            QueryPerformanceCounter(&end);
            difference.QuadPart = end.QuadPart - start.QuadPart;
        } while (difference.QuadPart < delta.QuadPart);
    }
}
