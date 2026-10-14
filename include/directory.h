#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "sorting.h"

/* Read directory contents into a dynamic array. */
int read_directory(const char *path,
                   const LSOptions *opts,
                   FileEntry **out,
                   size_t *count);

/* Display a directory, optionally recursively. */
int list_directory(const char *path,
                   const LSOptions *opts,
                   int header);

/* Release allocated directory entries. */
void free_entries(FileEntry *entries,
                  size_t count);

#endif

