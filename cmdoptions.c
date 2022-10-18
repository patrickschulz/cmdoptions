#include "cmdoptions.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define _MAX(a, b) ((a) > (b) ? (a) : (b))

struct option {
    char short_identifier;
    const char* long_identifier;
    int numargs;
    void* argument; /* is char* for once-only options, char** (with NULL terminator) for multiple options */
    int was_provided;
    const char* help;
    struct option* aliased;
};

struct section {
    char* name;
};

struct entry {
    void* value;
    enum { SECTION, OPTION } what;
};

struct cmdoptions {
    void** entries;
    size_t entries_size;
    size_t entries_capacity;
    char** positional_parameters;
    char* prehelpmsg;
    char* posthelpmsg;
    int force_narrow_mode;
};

struct cmdoptions* cmdoptions_create(void)
{
    struct cmdoptions* options = malloc(sizeof(*options));
    options->entries_size = 0;
    options->entries_capacity = 1;
    options->entries = malloc(options->entries_capacity * sizeof(*options->entries));
    options->positional_parameters = malloc(sizeof(*options->positional_parameters));;
    *options->positional_parameters = NULL;
    options->prehelpmsg = malloc(1);
    options->prehelpmsg[0] = 0;
    options->posthelpmsg = malloc(1);
    options->posthelpmsg[0] = 0;
    options->force_narrow_mode = 0;
    return options;
}

void cmdoptions_enable_narrow_mode(struct cmdoptions* options)
{
    options->force_narrow_mode = 1;
}

void cmdoptions_disable_narrow_mode(struct cmdoptions* options)
{
    options->force_narrow_mode = 0;
}

void _destroy_entry(void* ptr)
{
    struct entry* entry = ptr;
    if(entry->what == OPTION)
    {
        struct option* option = entry->value;
        if(option->argument)
        {
            if(option->numargs & MULTI_ARGS)
            {
                char** p = option->argument;
                while(*p)
                {
                    free(*p);
                    ++p;
                }
            }
            free(option->argument);
        }
    }
    else /* SECTION */
    {
        struct section* section = entry->value;
        free(section->name);
    }
    free(entry->value);
    free(ptr);
}

void cmdoptions_destroy(struct cmdoptions* options)
{
    unsigned int i;
    char** p;
    for(i = 0; i < options->entries_size; ++i)
    {
        _destroy_entry(options->entries[i]);
    }
    free(options->entries);
    p = options->positional_parameters;
    while(*p)
    {
        free(*p);
        ++p;
    }
    free(options->positional_parameters);
    free(options->prehelpmsg);
    free(options->posthelpmsg);
    free(options);
}

void cmdoptions_exit(struct cmdoptions* options, int exitcode)
{
    cmdoptions_destroy(options);
    exit(exitcode);
}

static int _check_capacity(struct cmdoptions* options)
{
    void** tmp;
    if(options->entries_size + 1 > options->entries_capacity)
    {
        options->entries_capacity *= 2;
        tmp = realloc(options->entries, sizeof(*tmp) * options->entries_capacity);
        if(!tmp)
        {
            return 0;
        }
        options->entries = tmp;
    }
    return 1;
}

static int _add_entry(struct cmdoptions* options, struct entry* entry)
{
    if(!_check_capacity(options))
    {
        return 0;
    }
    options->entries[options->entries_size] = entry;
    options->entries_size += 1;
    return 1;
}

int cmdoptions_add_section(struct cmdoptions* options, const char* name)
{
    struct entry* entry;
    struct section* section = malloc(sizeof(*section));
    section->name = malloc(strlen(name));
    if(!section->name)
    {
        free(section);
        return 0;
    }
    strcpy(section->name, name);
    entry = malloc(sizeof(*entry));
    entry->what = SECTION;
    entry->value = section;
    if(!_add_entry(options, entry))
    {
        free(section);
        free(entry);
        return 0;
    }
    return 1;
}

static struct entry* _create_option(char short_identifier, const char* long_identifier, int numargs, const char* help)
{
    struct entry* entry;
    struct option* option = malloc(sizeof(*option));
    if(!option)
    {
        return NULL;
    }
    option->short_identifier = short_identifier;
    option->long_identifier = long_identifier;
    option->numargs = numargs;
    option->argument = NULL;
    option->was_provided = 0;
    option->help = help;
    option->aliased = NULL;
    entry = malloc(sizeof(*entry));
    if(!entry)
    {
        free(option);
        return NULL;
    }
    entry->value = option;
    entry->what = OPTION;
    return entry;
}

int cmdoptions_add_alias(struct cmdoptions* options, const char* long_aliased_identifier, char short_identifier, const char* long_identifier, const char* help)
{
    struct entry* entry;
    struct option* alias = NULL;
    unsigned int i;
    for(i = 0; i < options->entries_size; ++i)
    {
        struct entry* entry = options->entries[i];
        if(entry->what == OPTION)
        {
            struct option* option = entry->value;
            if(strcmp(option->long_identifier, long_aliased_identifier) == 0)
            {
                alias = option;
                break;
            }
        }
    }

    entry = _create_option(short_identifier, long_identifier, 0, help); /* num_args will never be used */
    if(!entry)
    {
        return 0;
    }
    ((struct option*)entry->value)->aliased = alias;
    if(!_add_entry(options, entry))
    {
        _destroy_entry(entry);
        return 0;
    }
    return 1;
}

int cmdoptions_add_option(struct cmdoptions* options, char short_identifier, const char* long_identifier, int numargs, const char* help)
{
    struct entry* entry = _create_option(short_identifier, long_identifier, numargs, help);
    if(!_add_entry(options, entry))
    {
        _destroy_entry(entry);
        return 0;
    }
    return 1;
}


int cmdoptions_add_option_default(struct cmdoptions* options, char short_identifier, const char* long_identifier, int numargs, const char* default_arg, const char* help)
{
    struct entry* entry = _create_option(short_identifier, long_identifier, numargs, help);
    if(numargs > 1)
    {
        char** arg = calloc(2, sizeof(*arg));
        arg[0] = malloc(strlen(default_arg));
        strcpy(arg[0], default_arg);
        arg[1] = NULL;
        ((struct option*)entry->value)->argument = arg;
    }
    else
    {
        char* arg = malloc(strlen(default_arg));
        strcpy(arg, default_arg);
        ((struct option*)entry->value)->argument = arg;
    }
    if(!_add_entry(options, entry))
    {
        _destroy_entry(entry);
        return 0;
    }
    return 1;
}

void cmdoptions_prepend_help_message(struct cmdoptions* options, const char* msg)
{
    size_t len = strlen(options->prehelpmsg) + strlen(msg);
    char* str = realloc(options->prehelpmsg, len + 1);
    strcat(str, msg);
}

void cmdoptions_append_help_message(struct cmdoptions* options, const char* msg)
{
    size_t len = strlen(options->posthelpmsg) + strlen(msg);
    char* str = realloc(options->posthelpmsg, len + 1);
    strcat(str, msg);
}

static int _get_screen_width(unsigned int* width)
{
    struct winsize ws;
    int fd;

    fd = open("/dev/tty", O_RDWR);
    if(fd < 0 || ioctl(fd, TIOCGWINSZ, &ws) < 0)
    {
        err(8, "/dev/tty");
        return 0;
    }

    close(fd);

    *width = ws.ws_col;
    return 1;
}

static void _print_sep(unsigned int num)
{
    unsigned int i;
    for(i = 0; i < num; ++i)
    {
        putchar(' ');
    }
}

static void _put_line(unsigned int textwidth, unsigned int* linewidth, const char** ch, const char* wptr, unsigned int leftmargin)
{
    if(*linewidth + wptr - *ch > textwidth)
    {
        *linewidth = 0;
        putchar('\n');
        _print_sep(leftmargin - 1);
    }
    *linewidth += (wptr - *ch);
    while(*ch < wptr)
    {
        putchar(**ch);
        ++(*ch);
    }
}

static void _print_wrapped_paragraph(const char* text, unsigned int textwidth, unsigned int leftmargin)
{
    const char* ch = text;
    const char* wptr = ch;
    unsigned int linewidth = 0;
    while(*wptr)
    {
        if(*wptr == ' ')
        {
            _put_line(textwidth, &linewidth, &ch, wptr, leftmargin);
        }
        ++wptr;
    }
    _put_line(textwidth, &linewidth, &ch, wptr, leftmargin);
}

void cmdoptions_help(const struct cmdoptions* options)
{
    unsigned int displaywidth = 80;
    unsigned int optwidth = 0;
    unsigned int i;
    unsigned int startskip = 4;
    unsigned int helpsep = 4;
    unsigned int leftmargin = 0;
    unsigned int rightmargin = 1;
    int narrow;
    unsigned int offset;
    unsigned int textwidth;
    unsigned int count;

    _get_screen_width(&displaywidth);

    /* find maximum options width */
    for(i = 0; i < options->entries_size; ++i)
    {
        struct entry* entry = options->entries[i];
        if(entry->what == OPTION)
        {
            struct option* option = entry->value;
            if(option->short_identifier && !option->long_identifier)
            {
                optwidth = _MAX(optwidth, 2); /* 2: -%c */
            }
            else if(!option->short_identifier && option->long_identifier)
            {
                optwidth = _MAX(optwidth, strlen(option->long_identifier) + 2); /* + 2: -- */
            }
            else
            {
                optwidth = _MAX(optwidth, 2 + 1 + strlen(option->long_identifier) + 2); /* +1: , */
            }
        }
    }

    narrow = options->force_narrow_mode || (displaywidth < 100); /* FIXME: make dynamic (dependent on maximum word width or something) */

    offset = narrow ? 2 * startskip : optwidth + startskip + helpsep;
    textwidth = displaywidth - offset - leftmargin - rightmargin;

    puts(options->prehelpmsg);
    puts("list of command line options:\n");
    for(i = 0; i < options->entries_size; ++i)
    {
        struct entry* entry = options->entries[i];
        if(entry->what == SECTION)
        {
            struct section* section = entry->value;
            puts(section->name);
        }
        else
        {
            struct option* option = entry->value;
            _print_sep(startskip);
            count = optwidth;
            if(option->short_identifier)
            {
                putchar('-');
                putchar(option->short_identifier);
                count -= 2;
            }
            if(option->short_identifier && option->long_identifier)
            {
                putchar(',');
                count -= 1;
            }
            if(option->long_identifier)
            {
                putchar('-');
                putchar('-');
                fputs(option->long_identifier, stdout);
                count -= (2 + strlen(option->long_identifier));
            }
            if(narrow)
            {
                putchar('\n');
                _print_sep(2 * startskip);
            }
            else
            {
                _print_sep(helpsep + count);
            }
            leftmargin = narrow ? 2 * startskip : startskip + optwidth + helpsep;
            _print_wrapped_paragraph(option->help, textwidth, leftmargin);
            putchar('\n');
        }
    }
    puts(options->posthelpmsg);
}

static void _print_with_correct_escape_sequences(const char* str)
{
    unsigned int numescape = 0;
    const char* ptr = str;
    size_t len = strlen(str);
    char* buf;
    char* dest;

    /* count number of required escapes */
    while(*ptr)
    {
        if(*ptr == '\\')
        {
            ++numescape;
        }
        ++ptr;
    }

    /* assemble escaped string */
    buf = malloc(len + numescape + 1);
    ptr = str;
    dest = buf;
    while(*ptr)
    {
        *dest = *ptr;
        if(*ptr == '\\')
        {
            *(dest + 1) = '\\';
            ++dest;
        }
        ++ptr;
        ++dest;
    }
    *dest = 0;
    puts(buf);
    free(buf);
}

void cmdoptions_export_manpage(const struct cmdoptions* options)
{
    unsigned int i;
    for(i = 0; i < options->entries_size; ++i)
    {
        struct entry* entry = options->entries[i];
        if(entry->what == OPTION)
        {
            struct option* option = entry->value;
            fputs(".IP \"\\fB\\", stdout);
            if(option->short_identifier && option->long_identifier)
            {
                fputc('-', stdout);
                fputc(option->short_identifier, stdout);
                fputc(',', stdout);
                fputc('-', stdout);
                fputc('-', stdout);
                fputs(option->long_identifier, stdout);
            }
            else if(option->short_identifier)
            {
                fputc('-', stdout);
                fputc(option->short_identifier, stdout);
            }
            else if(option->long_identifier)
            {
                fputc('-', stdout);
                fputc('-', stdout);
                fputs(option->long_identifier, stdout);
            }
            printf("\\fR %s\" 4\n", "");
            _print_with_correct_escape_sequences(option->help);
        }
        else /* section */
        {
            struct section* section = entry->value;
            printf(".SS %s\n", section->name);
        }
    }
}

static struct option* _get_option(struct cmdoptions* options, char short_identifier, const char* long_identifier)
{
    unsigned int i;
    int found = 0;
    for(i = 0; i < options->entries_size; ++i)
    {
        struct entry* entry = options->entries[i];
        if(entry->what == OPTION)
        {
            struct option* option = entry->value;
            if(long_identifier)
            {
                found = (strcmp(option->long_identifier, long_identifier) == 0);
            }
            else
            {
                found = (option->short_identifier == short_identifier);
            }
            if(found)
            {
                if(option->aliased)
                {
                    return option->aliased;
                }
                else
                {
                    return option;
                }
            }
        }
    }
    return NULL;
}

const char** cmdoptions_get_positional_parameters(struct cmdoptions* options)
{
    return (const char**) options->positional_parameters;
}

int cmdoptions_no_args_given(const struct cmdoptions* options)
{
    unsigned int i;
    for(i = 0; i < options->entries_size; ++i)
    {
        const struct entry* entry = options->entries[i];
        if(entry->what == OPTION)
        {
            const struct option* option = entry->value;
            if(option->was_provided)
            {
                return 0;
            }
        }
    }
    return 1;
}

int cmdoptions_was_provided_short(struct cmdoptions* options, char short_identifier)
{
    const struct option* option = _get_option(options, short_identifier, NULL);
    if(option)
    {
        return option->was_provided;
    }
    return 0;
}

int cmdoptions_was_provided_long(struct cmdoptions* options, const char* long_identifier)
{
    const struct option* option = _get_option(options, 0, long_identifier);
    if(option)
    {
        return option->was_provided;
    }
    return 0;
}

int _store_argument(struct option* option, int* iptr, int argc, const char* const * argv)
{
    int j;
    int len;
    char** argument;
    if(option->numargs)
    {
        if(*iptr < argc - 1)
        {
            if(option->numargs & MULTI_ARGS)
            {
                if(!option->argument)
                {
                    argument = calloc(2, sizeof(char*));
                    argument[0] = malloc(strlen(argv[*iptr + 1]));
                    strcpy(argument[0], argv[*iptr + 1]);
                    option->argument = argument;
                }
                else
                {
                    char** ptr = option->argument;
                    while(*ptr) { ++ptr; }
                    len = ptr - (char**)option->argument;
                    argument = calloc(len + 2, sizeof(char*));
                    for(j = 0; j < len; ++j)
                    {
                        argument[j] = ((char**)option->argument)[j];
                    }
                    argument[len] = malloc(strlen(argv[*iptr + 1]));
                    strcpy(argument[len], argv[*iptr + 1]);
                    free(option->argument);
                    option->argument = argument;
                }
            }
            else /* SINGLE_ARG option */
            {
                option->argument = malloc(strlen(argv[*iptr + 1]));
                strcpy(option->argument, argv[*iptr + 1]);
            }
        }
        else /* argument required, but not entries in argv left */
        {
            if(option->long_identifier)
            {
                printf("expected argument for option '%s'\n", option->long_identifier);
            }
            else
            {
                printf("expected argument for option '%c'\n", option->short_identifier);
            }
            return 0;
        }
        *iptr += 1;
    }
    return 1;
}

int cmdoptions_parse(struct cmdoptions* options, int argc, const char* const * argv)
{
    int endofoptions = 0;
    int i;
    for(i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        if(!endofoptions && arg[0] == '-' && arg[1] == 0); /* single dash (-) */
        else if(!endofoptions && arg[0] == '-' && arg[1] == '-' && arg[2] == 0) /* end of options (--) */
        {
            endofoptions = 1;
        }
        else if(!endofoptions && arg[0] == '-') /* option */
        {
            if(arg[1] == '-') /* long option */
            {
                const char* longopt = arg + 2;
                struct option* option = _get_option(options, 0, longopt);
                if(!option)
                {
                    printf("unknown command line option: '--%s'\n", longopt);
                    return 0;
                }
                else
                {
                    if(option->was_provided && !(option->numargs & MULTI_ARGS))
                    {
                        printf("option '%s' is only allowed once\n", longopt);
                    }
                    option->was_provided = 1;
                    if(!_store_argument(option, &i, argc, argv))
                    {
                        return 0;
                    }
                }
            }
            else /* short option */
            {
                const char* ch = arg + 1;
                while(*ch)
                {
                    char shortopt = *ch;
                    struct option* option = _get_option(options, shortopt, NULL);
                    if(!option)
                    {
                        /*printf("unknown command line option: '--%s'\n", longopt); */
                        /*return 0; */
                    }
                    else
                    {
                        if(option->was_provided && !(option->numargs & MULTI_ARGS))
                        {
                            printf("option '%c' is only allowed once\n", shortopt);
                        }
                        option->was_provided = 1;
                        if(!_store_argument(option, &i, argc, argv))
                        {
                            return 0;
                        }
                    }
                    ++ch;
                }
            }
        }
        else /* positional parameter */
        {
            unsigned int count = 0;
            char** p = options->positional_parameters;
            while(*p)
            {
                ++count;
                ++p;
            }
            options->positional_parameters = realloc(options->positional_parameters, count + 1);
            options->positional_parameters[count] = malloc(strlen(arg));
            strcpy(options->positional_parameters[count], arg);
        }
    }
    return 1;
}

const void* cmdoptions_get_argument_short(struct cmdoptions* options, char short_identifier)
{
    const struct option* option = _get_option(options, short_identifier, NULL);
    if(option)
    {
        return option->argument;
    }
    return NULL;
}

const void* cmdoptions_get_argument_long(struct cmdoptions* options, const char* long_identifier)
{
    const struct option* option = _get_option(options, 0, long_identifier);
    if(option)
    {
        return option->argument;
    }
    return NULL;
}

