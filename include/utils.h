#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

#include "options.h"

/* Get the block reporting unit. */
uintmax_t get_block_unit(const LSOptions *opts);

/* Convert allocated 512-byte blocks. */
uintmax_t convert_blocks(uintmax_t blocks,
                         uintmax_t unit);

/* Format sizes using binary prefixes. */
void format_human(uintmax_t bytes,
                  char *buffer,
                  size_t size);

/* Format allocated filesystem blocks. */
void format_blocks(uintmax_t blocks,
                   const LSOptions *opts,
                   char *buffer,
                   size_t size);

/* Select mtime, ctime or atime. */
time_t selected_time(const struct stat *st,
                     TimeType type);

/* Allocate a combined directory/file path. */
char *join_path(const char *dir,
                const char *name);

#endif

