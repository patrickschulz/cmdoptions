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
    cmdoptions_add_option(cmdoptions, 'h', "help", NO_ARG, "display help");
    cmdoptions_add_option(cmdoptions, NO_SHORT, "number", SINGLE_ARG, "number");

    /* parse options */
    returnvalue = 0;
    if(!cmdoptions_parse(cmdoptions, argc, argv))
    {
        returnvalue = 1;
        goto DESTROY_CMDOPTIONS;
    }

    /* test for options */
    if(cmdoptions_was_provided_long(cmdoptions, "help"))
    {
        cmdoptions_help(cmdoptions); /* display help message */
        goto DESTROY_CMDOPTIONS;
    }

    /* option with argument */
    if(cmdoptions_was_provided_long(cmdoptions, "number"))
    {
        int num = atoi(cmdoptions_get_argument_long(cmdoptions, "number"));
        printf("number was: %d\n", num);
    }

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
