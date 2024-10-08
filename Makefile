MAKEFLAGS += --no-print-directory
SHELL := /bin/bash

build: clean
	mkdir -p build
	cd build; cmake ..; $(MAKE) -j8

clean:
	rm -rf build

.PHONY: build clean
