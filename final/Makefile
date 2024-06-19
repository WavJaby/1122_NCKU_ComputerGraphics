all: copyLib build

.PHONY: build

copyLib:
	mkdir -p build/
	cp -r ../lib ./
	cp -r ../external_lib/freetype/include/* ./lib
	cp -r ../external_lib/glad/include/* ./lib
	cp -r ../external_lib/glfw/include/* ./lib
	cp -r ../external_lib/stb_image/include/* ./lib
	cp -r ../external_lib/WJCL/include/* ./lib
	cp /usr/local/lib/libfreetype.so ./build

build:
	gcc -L/usr/lib/x86_64-linux-gnu/ -Ilib/ -g main.c -o build/main.out -lfreetype -lglfw3 -lGL -lm