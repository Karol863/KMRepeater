A simple program to simulate keyboard and mouse events for windows using low level hooks. Made with the goal of learning C.

## Building
* Install [MSYS2](https://www.msys2.org/), then install the clang compiler using this command `pacman -S mingw-w64-x86_64-clang`.

```console
clang -o KMRepeater.exe main.c -march=native -O2 -flto -pipe -s -D_FORTIFY_SOURCE=1
```
