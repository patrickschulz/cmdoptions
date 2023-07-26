cmdoptions_add_option(cmdoptions, 'h', "help", NO_ARG, "display help");
cmdoptions_add_mode(cmdoptions, "create");
cmdoptions_mode_add_option(cmdoptions, "create", 'n', "number", SINGLE_ARG, "number");
