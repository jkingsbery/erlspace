test:
	make build;

build:
	g++ -I"/usr/include/SDL" -c src/hello.c -o bin/hello.o;
	g++ bin/hello.o -lSDL -lSDL_image -o bin/hello;

run:
	bin/hello;

clean:
	-rm -rf bin;
	-rm hello.o;