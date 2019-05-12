#!/bin/bash
i686-w64-mingw32-g++ -c -o hello_world.o hello_world.cpp -D ADD_EXPORTS
i686-w64-mingw32-g++ -static-libstdc++ -static-libgcc -o hello_world.dll hello_world.o -s -shared -Wl,--out-implib,libhello_world.a
i686-w64-mingw32-g++ -c -o hello_text.o hello_text.c
i686-w64-mingw32-g++ -static-libstdc++ -static-libgcc -o hello_test.exe -s hello_text.o -L. -lhello_world
