.PHONY: all
all: example example.1

example: example.c cmdoptions.h cmdoptions.c cmdoptions_def.c
	gcc -g -Og -ansi -Wall -Wextra -pedantic example.c cmdoptions.c -o example

example.1: generate_manpage
	./generate_manpage > example.1

generate_manpage: manpage.c cmdoptions.h cmdoptions.c cmdoptions_def.c
	gcc -g -Og -ansi -Wall -Wextra -pedantic manpage.c cmdoptions.c -o generate_manpage

.PHONY: clean
clean:
	rm -f example generate_manpage
