# Command-Line Options Parsing in ANSI C
This repository provides a framework for parsing command-line options in C.
It's main goals area ease of use and flexibility as well as some advanced features such as a nice help display and manpage generation.
The parser features both a simple interface for command-line options like `program --foo -x -y` as well as a mode-driven interface like `git`, e.g. `program create --foo -x -y`.

# Usage
This is not intended as a library that you link but rather a source file in your own code. Copy `cmdoptions.{c,h}` and use them similar to as in `example.c` and `cmdoptions_def.c`.

# Compilation Flags
The help display takes the terminal width into account, which requires a few header files (sys/ioctl.h, err.h, fcntl.h, unistd.h and termios.h).
If you have these headers, set ENABLE_TERM_WIDTH on the compiler command line, otherwise the width is always just assumed to be 80.

# API
## Initialization
- `cmdoptions_create`
This creates the `cmdoptions` structure, which is passed as first argument to all API functions.
- `cmdoptions_enable_narrow_mode`
- `cmdoptions_disable_narrow_mode`
For widths smaller than 100, a narrower help display is used. This function can be called to enforce this modes for all terminal widths.
- `cmdoptions_destroy`
This function cleans up all related data.
- `cmdoptions_exit`
This is a convenience function, which acts like `exit`, but call `cmdoptions_destroy` first.
- `cmdoptions_is_valid`
Check if the cmdoptions structure is valid. It can become invalid when certain actions fail, such as prepending to help messages.
- `cmdoptions_all_options_checked`
Check if all defined options are actually checked by the code. This function can/should be called after parsing and checking which options where provided. This is a useful sanity check in case there are many options (where it is easy to forget something).

## Adding Options
- `cmdoptions_add_section`
Add a section to the command-line options. All options added after this call belong to this section. This only affects the help display and has no functional impact.
- `cmdoptions_add_option`
Add a command-line option. Required arguments are a short and a long identifier, a flag indicating the number of arguments and help message.
In case of only-short options, the long identifier can be `NULL`, in case of only-long options the short identifier can be `0`.
The number-of-arguments flag should be one of `NO_ARG`, `SINGLE_ARG` or `MULTI_ARGS`.
- `cmdoptions_add_option_default`
Add a command-line option with a default argument. This works like `cmdoptions_add_option`, but in case the option is not given on the command line, the default value will be returned by `cmdoptions_get_argument_short` and `cmdoptions_get_argument_long`.
- `cmdoptions_add_alias`
Add an alias for another option. This takes the long identifier of the other option (which must be defined earlier) and adds an option (with possibly both short and long identifiers).
- `cmdoptions_add_mode`
Add a new mode. This groups command-line options to a common command, e.g. like `git` does.
- `cmdoptions_mode_add_section`
This works like `cmdoptions_add_section`, but for a specific mode.
- `cmdoptions_mode_add_option`
This works like `cmdoptions_add_option`, but for a specific mode.
- `cmdoptions_mode_add_option_default`
This works like `cmdoptions_add_option_default`, but for a specific mode.
- `cmdoptions_mode_add_alias`
You'd expect this function to exist, but, surprisingly even to me, it does not. It's not hard to implement and I might some time in the future. Then again, aliases are not particularly important.


## Help Message
- `cmdoptions_prepend_help_message`
Prepend arbitrary strings to the help message, that is, these are displayed first.
Multiple calls *append* to this message, so that they are displayed in the sequence they where added (but still before the main help message).
- `cmdoptions_append_help_message`
Append arbitrary strings to the help message, that is, these are displayed last.
Multiple calls *append* to this message, so that they are displayed in the sequence they where added (and after the main help message).

## Parsing
- `cmdoptions_parse`
Parse the command-line options. Takes `argc` and `argv` as arguments.

## Info
- `cmdoptions_help`
Display the main help message. This function does automated formatting based on the terminal size including paragraph wrapping.
- `cmdoptions_export_manpage`
Export all command-line options in a manpage format. The surrounding text has to be written, see `manpage.c`.

## Check/Retrieve Options
- `cmdoptions_empty`
Returns 1 if the parsed command line was empty.
- `cmdoptions_no_args_given`
Returns 1 if no options where given. Positional parameters that are not options (such as `program foo bar`) are possible.
- `cmdoptions_mode_no_args_given`
Returns 1 if not options where given for this mode. As for `cmdoptions_no_args_given`, positional parameters do no count.

- `cmdoptions_was_provided_short`, `cmdoptions_was_provided_long`, `cmdoptions_mode_was_provided_short` and `cmdoptions_mode_was_provided_long`
Return a flag (`0` or `1`) if the option was provided. Takes either a `char` (short) or a `const char*` (long).
The mode versions take the `mode` as second argument as a `const char*`.

- `cmdoptions_get_argument_short`, `cmdoptions_get_argument_long`, `cmdoptions_mode_get_argument_short` and `cmdoptions_mode_get_argument_long`
Retrive the given value for the respective option. The option can be identified by either the short identifier (`char`) or the long one `(const char*)`.
All four functions return `const void*`, which is either `const char*` or `const char**`, depending on whether the option has a single argument or multiple arguments.
Multiple arguments are `NULL` terminated at the end.

- `cmdoptions_get_positional_parameters`
Returns a `const char**` with a sentinel (`NULL` terminator at the end), so you can loop over all positional parameters with a simple `while`-loop.

# License
MIT No Attribution License

Essentially, take the code and copy/modify it as needed. No attribution is required (I don't mind if you still do).

