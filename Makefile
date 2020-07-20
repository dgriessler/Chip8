CC=gcc
CCFLAGS=-lallegro -lallegro_font -lallegro_audio -lallegro_acodec

default_target: all
all: main.c chip8.c
	$(CC) -o main main.c chip8.c $(CCFLAGS)

clean:
	del main.exe
