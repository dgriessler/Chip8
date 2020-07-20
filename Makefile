CC=C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin\gcc.exe
CCFLAGS=-lallegro -lallegro_font -lallegro_audio -lallegro_acodec

default_target: all
all: main.c chip8.c
	$(CC) -o main main.c chip8.c $(CCFLAGS)

clean:
	del main.exe
