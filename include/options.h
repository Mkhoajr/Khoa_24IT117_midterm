#ifndef OPTIONS_H
#define OPTIONS_H

/*
 * Available sorting methods.
 */
typedef enum {
    SORT_NAME,
    SORT_SIZE,
    SORT_TIME,
    SORT_NONE
} SortType;

/*
 * Output formats.
 */
typedef enum {
    FORMAT_SHORT,
    FORMAT_LONG,
    FORMAT_NUMERIC
} OutputFormat;

/*
 * File timestamp selection.
 */
typedef enum {
    TIME_MODIFIED,
    TIME_CHANGED,
    TIME_ACCESSED
} TimeType;

/*
 * Size formatting modes.
 */
typedef enum {
    SIZE_DEFAULT,
    SIZE_KILO,
    SIZE_HUMAN
} SizeType;

/*
 * Filename output modes.
 */
typedef enum {
    NAME_AUTO,
    NAME_QUESTION,
    NAME_RAW
} NameFormat;

/*
 * Options selected from the command line.
 */
typedef struct {
    int show_all;
    int almost_all;
    int directory_only;
    int recursive;

    int reverse;
    int show_inode;
    int classify;
    int show_blocks;

    SortType sort_type;
    OutputFormat format;
    TimeType time_type;
    SizeType size_type;
    NameFormat name_format;
} LSOptions;

/*
 * Parse command-line options.
 */
int parse_options(int argc, char *argv[],
                  LSOptions *opts);

#endif

