A simple program to simulate keyboard and mouse events for windows using low level hooks. Made with the goal of learning C.

## Building
* Install [MSYS2](https://www.msys2.org/), then install the gcc compiler using this command `pacman -S mingw-w64-ucrt-x86_64-gcc`.

```console
gcc -o KMRepeater.exe main.c -Wall -Wextra -Wwrite-strings -Wno-unused-result -march=native -O2 -ftree-vectorize -fno-semantic-interposition -fno-plt -pipe -s -flto -D_FORTIFY_SOURCE=2
```
