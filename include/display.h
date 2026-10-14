#ifndef DISPLAY_H
#define DISPLAY_H

#include "sorting.h"

/*
 * Column widths used by long-format output.
 */
typedef struct {
    size_t links;
    size_t owner;
    size_t group;
    size_t size;
    size_t inode;
    size_t blocks;
} DisplayWidths;

/* Calculate output column widths. */
void calculate_widths(const FileEntry *entries,
                      size_t count,
                      const LSOptions *opts,
                      DisplayWidths *widths);

/* Print one entry. */
int print_entry(const FileEntry *entry,
                const LSOptions *opts,
                const DisplayWidths *widths);

/* Print total directory size/blocks. */
void print_total(const FileEntry *entries,
                 size_t count,
                 const LSOptions *opts);

/* Print a filename according to -q/-w. */
void print_name(const char *name,
                const LSOptions *opts);

#endif

