cmdoptions_add_option(cmdoptions, 'x', "xoption", NO_ARG, "test option");
cmdoptions_add_mode(cmdoptions, "create");
cmdoptions_mode_add_option(cmdoptions, "create", 'n', "number", SINGLE_ARG, "number");
cmdoptions_prepend_help_message(cmdoptions, "*** Beginning of help message ***");
cmdoptions_append_help_message(cmdoptions, "*** End of help message ***");
