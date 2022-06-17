# TypeRacer
## Compiling
Simply compiling the .c files and linking them all will do, make sure to link -pthread.
```c
clang -static -O3 -o racer *.c -pthread
```
For server.
```c
clang -O3 -o server *.c -pthread
```
# Assets
Server needs a file called paragraphs.txt in the present working directory (usually same directory as executable).

This file needs to have one paragraph per line, each line must end with a line feed, with no preceeding carriage return.

This includes the last line.
