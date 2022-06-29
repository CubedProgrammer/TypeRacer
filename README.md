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
## Compatibility
Server is only designed to run on linux and there are no plans to make it run on anything else.

Client is tested to work on linux, in theory works on the varius BSD operating systems, and the Mac Os.

Support for windows is being slowly added, even though there is almost no point, seeing as modern windows comes preinstalled with linux.
## Assets
Server needs a file called paragraphs.txt in the present working directory (usually same directory as executable).

This file needs to have one paragraph per line, each line must end with a line feed, with no preceeding carriage return.

This includes the last line.
