SRC = src/rmlib.c

SO_TARGET = build/rmlib.so
DLL_TARGET = build/rmlib.dll

CC = gcc
CFLAGS = -O2 -Wall -fPIC

.PHONY: all linux windows clean

all: linux windows

linux:
	$(CC) $(CFLAGS) -shared -o $(SO_TARGET) $(SRC)

windows:
	x86_64-w64-mingw32-gcc -O2 -Wall -shared -o $(DLL_TARGET) $(SRC) -Wl,--out-implib,libmylib.a

clean:
	rm -f $(SO_TARGET) $(DLL_TARGET) libmylib.a
