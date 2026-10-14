#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

/*
 * Parse ls command-line arguments.
 * Return the first operand index.
 */
int parse_options(int argc, char *argv[],
                  LSOptions *opts)
{
    int ch;

    memset(opts, 0, sizeof(*opts));

    opts->sort_type = SORT_NAME;
    opts->format = FORMAT_SHORT;
    opts->time_type = TIME_MODIFIED;
    opts->size_type = SIZE_DEFAULT;
    opts->name_format = NAME_AUTO;

    opterr = 0;

    while ((ch = getopt(argc, argv,
                        "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            opts->almost_all = 1;
            break;

        case 'a':
            opts->show_all = 1;
            break;

        case 'c':
            opts->time_type = TIME_CHANGED;
            break;

        case 'd':
            opts->directory_only = 1;
            opts->recursive = 0;
            break;

        case 'F':
            opts->classify = 1;
            break;

        case 'f':
            opts->sort_type = SORT_NONE;
            opts->show_all = 1;
            break;

        case 'h':
            opts->size_type = SIZE_HUMAN;
            break;

        case 'i':
            opts->show_inode = 1;
            break;

        case 'k':
            opts->size_type = SIZE_KILO;
            break;

        case 'l':
            opts->format = FORMAT_LONG;
            break;

        case 'n':
            opts->format = FORMAT_NUMERIC;
            break;

        case 'q':
            opts->name_format = NAME_QUESTION;
            break;

        case 'R':
            opts->recursive = 1;
            opts->directory_only = 0;
            break;

        case 'r':
            opts->reverse = 1;
            break;

        case 'S':
            opts->sort_type = SORT_SIZE;
            break;

        case 's':
            opts->show_blocks = 1;
            break;

        case 't':
            opts->sort_type = SORT_TIME;
            break;

        case 'u':
            opts->time_type = TIME_ACCESSED;
            break;

        case 'w':
            opts->name_format = NAME_RAW;
            break;

        default:
            fprintf(stderr,
                    "myls: invalid option -- '%c'\n",
                    optopt ? optopt : '?');
            return -1;
        }
    }

    return optind;
}

