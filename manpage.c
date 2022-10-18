#include <stdio.h>

#include "cmdoptions.h"

int main(void)
{
    struct cmdoptions* cmdoptions = cmdoptions_create();
    #include "cmdoptions_def.c"
    puts(".TH example 1 \"01 Jan 1970\" \"1.0\" \"example man page\"");
    puts(".SH NAME");
    puts("example \\- manpage example for cmdoptions");
    puts(".SH SYNOPSIS");
    puts("example"); /* put most-used options here */
    puts(".SH DESCRIPTION");
    puts(".B example ");
    puts("shows how to use cmdoptions to parse and process command-line options in C");
    cmdoptions_export_manpage(cmdoptions);
    puts(".SH AUTHOR");
    puts("Patrick Kurth <p.kurth@posteo.de>");

    cmdoptions_destroy(cmdoptions);
    return 0;
}

