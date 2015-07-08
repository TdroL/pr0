lib:
	g++ -Winline -Wextra -Wfatal-errors -static -m64 -std=c++14 -march=native -O3 -Iinclude/ -Icpp/main/ -c src/minball.cpp -o lib/minball.o
	ar cr lib/libminball.a lib/minball.o

test:
	g++ -Winline -Wextra -Wfatal-errors -static -m64 -std=c++14 -march=native -O3 -Iinclude/ -Llib/ src/test.cpp -lminball -o test.exe
	./test.exe

.PHONY: lib test