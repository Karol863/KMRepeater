A simple program to simulate keyboard and mouse events for windows using low level hooks.

## Building
* Get the [Clang MinGW](https://github.com/mstorsjo/llvm-mingw) toolchain, and compile the program using the command below.

```console
clang -o KMRepeater.exe main.c -O2 -flto -pipe -s -D_FORTIFY_SOURCE=1
```

## How to use
At first, read the `config.h` file, and configure it if needed. The user is supposed to first record, then save. After saving, you should see a `data` file, which has all the events you've recorded. If you want to replay from the `data` file, then simply use your replay key combination.
