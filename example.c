#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmdoptions.h"

int main(int argc, const char * const *argv)
{
    int returnvalue;
    const char** pos;
    unsigned int count;

    /* create state and add options */
    struct cmdoptions* cmdoptions = cmdoptions_create();
    #include "cmdoptions_def.c"
    if(!cmdoptions_is_valid(cmdoptions))
    {
        fprintf(stderr, "%s\n", "initialization of command-line options parser failed");
        cmdoptions_exit(cmdoptions, 1);
    }

    /* parse options */
    returnvalue = 0;
    if(!cmdoptions_parse(cmdoptions, argc, argv))
    {
        returnvalue = 1;
        fprintf(stderr, "%s\n", "parsing of command-line options failed");
        goto DESTROY_CMDOPTIONS;
    }

    /* test for options */
    if(cmdoptions_was_provided_long(cmdoptions, "help") || cmdoptions_empty(cmdoptions))
    {
        cmdoptions_help(cmdoptions); /* display help message */
        goto DESTROY_CMDOPTIONS;
    }

    /* option with argument */
    if(cmdoptions_mode_was_provided_long(cmdoptions, "create", "number"))
    {
        int num = atoi(cmdoptions_get_argument_long(cmdoptions, "number"));
        printf("number was: %d\n", num);
    }

    /* check that no options stay unprocessed */
    assert(cmdoptions_all_options_checked(cmdoptions));

    pos = cmdoptions_get_positional_parameters(cmdoptions);
    count = 1;
    while(*pos)
    {
        printf("positional parameter #%d: %s\n", count, *pos);
        ++count;
        ++pos;
    }

DESTROY_CMDOPTIONS:
    cmdoptions_destroy(cmdoptions);
    return returnvalue;
}
