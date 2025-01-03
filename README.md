A simple program to simulate keyboard and mouse events for windows using low level hooks. Made with the goal of learning C.

## Building
* Download the clang compiler from [llvm-mingw](https://github.com/mstorsjo/llvm-mingw), and compile the program using the command below.

```console
clang -o KMRepeater.exe main.c -march=native -O2 -flto -pipe -s -D_FORTIFY_SOURCE=1
```
