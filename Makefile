.PHONY: build
#mongo_flags = $(shell pkg-config --libs --cflags libmongoc-1.0)

build:
	mkdir -p build/
	gcc -o build/server server.c child.c #$(mongo_flags)
