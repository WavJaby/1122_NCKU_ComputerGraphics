

all: copyLib build

.PHONY: build

copyLib:
	cp -r ../lib ./
	cp -r ../../freetype-2.10.0/include/* ./lib
	mkdir -p build/

build:
	gcc -L../../freetype-2.10.0/objs/ -L/usr/lib/x86_64-linux-gnu/ -I/usr/include/ -Ilib -g main.c -o build/main.out -lfreetype -lglut -lGLU -lGL -lm