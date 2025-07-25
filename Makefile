CC= gcc
#CFLAGS= -std=c99 -O3 -Wall -Wextra -Wstrict-prototypes
CFLAGS= -ansi -O3 -Wall -Wextra -Wstrict-prototypes
ifeq ($(DEBUG), 1)
    CFLAGS= -ansi -O0 -g -fanalyzer -Wno-analyzer-deref-before-check -fsanitize=address -fsanitize=leak -fsanitize=undefined -Wall -Wextra -Werror=return-type -Werror=implicit-function-declaration -Werror=int-conversion -Werror=discarded-qualifiers -Wstrict-prototypes
endif

.PHONY: all
all: example example.1

example: example.c cmdoptions.h cmdoptions.c cmdoptions_def.c print.c print.h
	$(CC) $(CFLAGS) example.c cmdoptions.c print.c -o example -DCMDOPTIONS_ENABLE_TERM_WIDTH

example.1: generate_manpage
	./generate_manpage > example.1

generate_manpage: manpage.c cmdoptions.h cmdoptions.c cmdoptions_def.c print.c print.h
	$(CC) $(CFLAGS) manpage.c cmdoptions.c print.c -o generate_manpage

.PHONY: clean
clean:
	rm -f example generate_manpage
