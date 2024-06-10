glad=-I../external_lib/glad/include

all: copyLib build

.PHONY: build

copyLib:
	mkdir -p build/
	cp -r ../lib ./

build:
	gcc -L/usr/lib/x86_64-linux-gnu/ -I/usr/include/ -Ilib/ -Wall -g main.c -o build/main.out $(glad) -lglfw3 -lGL -lm