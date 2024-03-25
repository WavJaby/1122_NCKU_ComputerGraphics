

all: copyLib build


copyLib:
	cp -r ../lib ./

build:
	gcc -L/usr/lib/x86_64-linux-gnu/ -I/usr/include/ -g main.c -o main.out -lglut -lGLU -lGL -lm