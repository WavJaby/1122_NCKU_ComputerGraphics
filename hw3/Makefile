

all: copyLib build

.PHONY: build

copyLib:
	cp -r ../lib ./
	cp -r ../../freetype-2.10.0/include/* ./lib
	cp -r ../util3d/* ./lib
	mkdir -p build/

build:
	UTIL3D=./lib/math3d.c
	gcc -L../../freetype-2.10.0/objs/ -L/usr/lib/x86_64-linux-gnu/ -I/usr/include/ -Ilib -g main.c $(UTIL3D) -o build/main.out -lfreetype -lglut -lGLU -lGL -lm