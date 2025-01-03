#include "base.h"

// Configure how much memory you want to reserve for the program.
// The number MUST be a power of 2, otherwise the program will crash.
// GB-Bytes converter: https://www.unitconverters.net/data-storage/gigabyte-to-byte.htm
#define RESERVED_MEMORY 34359738368

// Configure what key combination you want to use to record, replay and stop after the current replay.
// Both RECORD and REPLAY MUST have a modifier, although STOP can't have one.
// Available modifiers: MOD_ALT, MOD_CONTROL, MOD_SHIFT and MOD_WIN
// Available keys: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define RECORD_MODIFIER  MOD_ALT
#define RECORD_KEY       VK_F8

#define REPLAY_MODIFIER  MOD_ALT 
#define REPLAY_KEY       VK_F12

#define STOP_KEY         VK_F4

// Configure the speed of replays.
// The value MUST end with an "f".
#define SPEED 1.0f

// Configure how long you want to wait before performing the replay.
// The time is in seconds.
#define TIME 10
