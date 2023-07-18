default:
	mkdir -p build/
	gcc server.c child.c -o build/server
