# Command-Line Options Parsing in ANSI C
This repository provides a framework for parsing command-line options in C.
It's main goals area ease of use and flexibility as well as some advanced features such as nice help display and manpage generation.

# Usage
This is not intended as a library that you link but rather a source file in your own code. Copy `cmdoptions.{c,h}` and use them similar to as in `example.c`.

# Compilation Flags
The help display takes the terminal width into account, which requires a few header files (sys/ioctl.h, err.h, fcntl.h, unistd.h and termios.h).
If you have these headers, set ENABLE_TERM_WIDTH on the compiler command line, otherwise the width is always just assumed to be 80.

# API
## Initialization
- `cmdoptions_create`
- `cmdoptions_disable_narrow_mode`
- `cmdoptions_destroy`
- `cmdoptions_exit`

## Adding Options
- `cmdoptions_add_section`
- `cmdoptions_add_option`
- `cmdoptions_add_option_default`
- `cmdoptions_add_alias`

## Help Message
- `cmdoptions_prepend_help_message`
- `cmdoptions_append_help_message`

## Parsing
- `cmdoptions_parse`

## Info
- `cmdoptions_help`

Display the main help message. This function does automated formatting based on the terminal size including paragraph wrapping.

- `cmdoptions_export_manpage`

Export all command-line options in a manpage format. The surrounding text has to be written, see `manpage.c`.

## Check/Retrieve Options
- `cmdoptions_was_provided_short` and `cmdoptions_was_provided_long`

Return a flag (`0` or `1`) if the option was provided.

- `cmdoptions_get_argument_short` and `cmdoptions_get_argument_long`

Both functions return `const void*`, which is either `const char*` or `const char**`, depending on whether the option has a single argument or multiple arguments.
Multiple arguments are `NULL` terminated at the end.

- `cmdoptions_get_positional_parameters`

Returns a `const char**` with a sentinel (`NULL` terminator at the end), so you can loop over all positional parameters with a simple `while`-loop.

# License
MIT No Attribution License

Essentially, take the code and copy/modify it as needed. No attribution is required (I don't mind if you still do).
