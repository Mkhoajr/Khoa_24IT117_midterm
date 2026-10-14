#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#include "utils.h"

/*
 * Get the reporting block size.
 */
uintmax_t get_block_unit(const LSOptions *opts)
{
    long block_size = 512;

    if (opts->size_type == SIZE_KILO)
        return 1024;

    if (opts->size_type == SIZE_HUMAN)
        return 512;

#ifdef __NetBSD__
    (void)getbsize(NULL, &block_size);
#else
    const char *env = getenv("BLOCKSIZE");

    if (env != NULL && *env != '\0') {
        char *end;

        long n = strtol(env, &end, 10);

        if (end != env && n > 0)
            block_size = n;
    }
#endif

    if (block_size > 0)
        return (uintmax_t)block_size;

    return 512;
}

/*
 * Convert allocated 512-byte blocks to the
 * requested block unit, rounding upward.
 */
uintmax_t convert_blocks(uintmax_t blocks,
                         uintmax_t unit)
{
    uintmax_t whole;
    uintmax_t remainder;
    uintmax_t left_bytes;

    if (unit == 0)
        return 0;

    whole = blocks / unit;
    remainder = blocks % unit;

    if (whole > UINTMAX_MAX / 512)
        return UINTMAX_MAX;

    left_bytes = remainder * 512;

    return whole * 512 +
           left_bytes / unit +
           (left_bytes % unit != 0);
}

/*
 * Format bytes using binary prefixes.
 */
void format_human(uintmax_t bytes,
                  char *buffer,
                  size_t size)
{
    static const char *units[] = {
        "B", "K", "M", "G", "T", "P", "E"
    };

    double value = (double)bytes;
    unsigned i = 0;

    while (value >= 1024.0 && i < 6) {
        value /= 1024.0;
        i++;
    }

    if (i == 0) {
        snprintf(buffer, size,
                 "%juB", bytes);
    } else if (value < 10.0 &&
               value != (double)(uintmax_t)value) {
        snprintf(buffer, size,
                 "%.1f%s", value, units[i]);
    } else {
        snprintf(buffer, size,
                 "%.0f%s", value, units[i]);
    }
}

/*
 * Format allocated filesystem blocks.
 */
void format_blocks(uintmax_t blocks,
                   const LSOptions *opts,
                   char *buffer,
                   size_t size)
{
    if (opts->size_type == SIZE_HUMAN) {
        if (blocks > UINTMAX_MAX / 512) {
            snprintf(buffer, size, "overflow");
            return;
        }

        format_human(
            blocks * 512,
            buffer,
            size);
    } else {
        snprintf(
            buffer,
            size,
            "%ju",
            convert_blocks(
                blocks,
                get_block_unit(opts)));
    }
}

/*
 * Select mtime, ctime, or atime.
 */
time_t selected_time(const struct stat *st,
                     TimeType type)
{
    if (type == TIME_CHANGED)
        return st->st_ctime;

    if (type == TIME_ACCESSED)
        return st->st_atime;

    return st->st_mtime;
}

/*
 * Combine a directory path and entry name.
 *
 * Caller must free the returned string.
 */
char *join_path(const char *dir,
                const char *name)
{
    size_t a = strlen(dir);
    size_t b = strlen(name);

    if (b > SIZE_MAX - 2 ||
        a > SIZE_MAX - b - 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    char *p = malloc(a + b + 2);

    if (p != NULL) {
        snprintf(
            p,
            a + b + 2,
            "%s/%s",
            dir,
            name);
    }

    return p;
}

