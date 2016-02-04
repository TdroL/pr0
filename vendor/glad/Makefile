lib:
	gcc -Winline -Wextra -Wfatal-errors -static -m64 -std=c11 -march=native -O3 -Iinclude/ -c src/glad.c -o lib/glad.o
	ar cr lib/libglad.a lib/glad.o
	rm lib/glad.o
.PHONY: lib