#include "functions.c"

static BOOL recording = FALSE;
static HHOOK mouse_hook;
static HHOOK keyboard_hook;
static ULONGLONG timer_start;
static ULONGLONG timer_end;

int main(void) {
    array_init(&arr);
    char number[4096];

    cxscreen = GetSystemMetrics(SM_CXSCREEN);
    cyscreen = GetSystemMetrics(SM_CYSCREEN);

	RegisterHotKey(NULL, 1, RECORD_MODIFIER, RECORD_KEY);
	RegisterHotKey(NULL, 2, REPLAY_MODIFIER, REPLAY_KEY);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (msg.message == WM_HOTKEY && msg.wParam == 1) {
			if (!recording) {
				mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_callback, NULL, 0);
				keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);

				recording = TRUE;
				timer_start = GetTickCount64();
				puts("Recording started. Click your recording key combination again to stop and save.");
			} else {
				timer_end = GetTickCount64();
				ULONGLONG difference = timer_end - timer_start;
				ULONGLONG totalSeconds = difference / 1000;
				ULONGLONG hours = totalSeconds / 3600;
				ULONGLONG minutes = (totalSeconds % 3600) / 60;
				ULONGLONG seconds = totalSeconds % 60;

				UnhookWindowsHookEx(mouse_hook);
				UnhookWindowsHookEx(keyboard_hook);

				FILE *f = fopen("data", "wb");
				if (f == NULL) {
					fputs("Error: failed to open file for writing!\n", stderr);
					goto cleanup;
				}

				fwrite(&arr.offset, sizeof(u64), 1, f);
				fwrite(arr.data, sizeof(EventInput), arr.offset, f);
				if (ferror(f)) {
					fputs("Error: failed to write data into the data file!\n", stderr);
					goto cleanup;
					fclose(f);
				}
				fclose(f);

				printf("Record saved. You were recoding for %llu hour(s), %llu minute(s), and %llu second(s).\n", hours, minutes, seconds);
				break;
			}
		} else if (msg.message == WM_HOTKEY && msg.wParam == 2) {
			arr.data = 0;
			arr.offset = 0;
			arr.capacity = 0;

			FILE *f = fopen("data", "rb");
			if (f == NULL) {
				fputs("Error: failed to open file for reading!\n", stderr);
				goto cleanup;
			}

			fread(&arr.offset, sizeof(u64), 1, f);
			array_alloc(&arr, arr.offset);
			fread(arr.data, sizeof(EventInput), arr.offset, f);
			if (ferror(f)) {
				fputs("Error: failed to read data from the data file!\n", stderr);
				goto cleanup;
				fclose(f);
			}
			fclose(f);

			printf("How many times you want to replay? In case of an infinite replay, enter 0: ");
			if (fgets(number, sizeof(number), stdin) == NULL) {
				fputs("Error: failed to read user input!\n", stderr);
				goto cleanup;
			}
			printf("Choosen number: %s\n", number);

			u16 times = atoi(number);
			if (times == 0) {
				u64 i;
				if (TIME > 0) {
					printf("Waiting %d seconds...\n", TIME);
					Sleep(TIME * 1000);
				}
				while (replaying != FALSE) {
					replay(SPEED);
					printf("Replay passed: %llu\n", i + 1);
					i += 1;
				}
			} else {
				u16 i;
				if (TIME > 0) {
					printf("Waiting %d seconds...\n", TIME);
					Sleep(TIME * 1000);
				}
				while (i < times && replaying != FALSE) {
					replay(SPEED);
					printf("Replay passed: %d out of: %d\n", i + 1, times);
					i += 1;
				}
			}

			puts("Replaying finished.");
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    UnregisterHotKey(NULL, 1);
	UnregisterHotKey(NULL, 2);
    array_free(&arr);
	return 0;

    cleanup:
        UnregisterHotKey(NULL, 1);
        UnregisterHotKey(NULL, 2);
        array_free(&arr);
		return 1;
}
