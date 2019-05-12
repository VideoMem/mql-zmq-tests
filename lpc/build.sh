#!/bin/bash
i686-w64-mingw32-g++ -c -o ./bin/lpclient.o lpclient.cpp -D ADD_EXPORTS
i686-w64-mingw32-g++ -static-libstdc++ -static-libgcc -o ./bin/lpclient.dll ./bin/lpclient.o -s -shared -Wl,--out-implib,./bin/liblpclient.a
i686-w64-mingw32-g++ -c -o ./bin/lpclient_test.o lpclient_test.cpp
i686-w64-mingw32-g++ -static-libstdc++ -static-libgcc -o ./bin/lpclient_test.exe -s ./bin/lpclient_test.o -L. -l./bin/lpclient
