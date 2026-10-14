#ifndef SORTING_H
#define SORTING_H

#include <stddef.h>
#include <sys/stat.h>

#include "options.h"

/*
 * Information about one filesystem entry.
 */
typedef struct {
    char *name;
    char *path;

    struct stat st;

    /* Distinguish directory operands. */
    int is_dir_operand;
} FileEntry;

/* Sort entries inside one directory. */
void sort_entries(FileEntry *entries,
                  size_t count,
                  const LSOptions *opts);

/* Sort and group command-line operands. */
void sort_operands(FileEntry *entries,
                   size_t count,
                   const LSOptions *opts);

#endif

