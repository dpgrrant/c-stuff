shell: main.c
	gcc -std=gnu99 -Wall -pedantic -O3 -march=native -o shell main.c

clean:
	rm shell
