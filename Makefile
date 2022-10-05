example: example.c cmdoptions.h cmdoptions.c
	gcc -g -Og -ansi -Wall -Wextra example.c cmdoptions.c -o example

.PHONY: clean
clean:
	rm example
