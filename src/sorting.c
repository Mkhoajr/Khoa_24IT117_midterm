#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sorting.h"

/*
 * qsort() does not accept a context argument.
 * This program uses a single-threaded sorter.
 */
static SortType sort_mode;
static TimeType time_mode;
static int reverse_order;

/* Compare the selected file timestamps. */
static int compare_time(const struct stat *a,
                        const struct stat *b)
{
    struct timespec ta;
    struct timespec tb;

    switch (time_mode) {
    case TIME_CHANGED:
        ta = a->st_ctim;
        tb = b->st_ctim;
        break;

    case TIME_ACCESSED:
        ta = a->st_atim;
        tb = b->st_atim;
        break;

    case TIME_MODIFIED:
    default:
        ta = a->st_mtim;
        tb = b->st_mtim;
        break;
    }

    if (ta.tv_sec != tb.tv_sec)
        return ta.tv_sec > tb.tv_sec ? -1 : 1;

    if (ta.tv_nsec != tb.tv_nsec)
        return ta.tv_nsec > tb.tv_nsec ? -1 : 1;

    return 0;
}

/* Compare two filesystem entries. */
static int compare_body(const FileEntry *a,
                        const FileEntry *b)
{
    int result = 0;

    if (sort_mode == SORT_SIZE) {
        if (a->st.st_size != b->st.st_size) {
            result =
                a->st.st_size > b->st.st_size
                ? -1 : 1;
        }
    } else if (sort_mode == SORT_TIME) {
        result = compare_time(
            &a->st, &b->st);
    }

    /* Use the filename as a tie-breaker. */
    if (result == 0) {
        result = strcmp(a->name, b->name);
    }

    return reverse_order ? -result : result;
}

/* qsort() adapter for directory entries. */
static int cmp_entries(const void *a,
                       const void *b)
{
    return compare_body(a, b);
}

/* qsort() adapter for path operands. */
static int cmp_operands(const void *va,
                        const void *vb)
{
    const FileEntry *a = va;
    const FileEntry *b = vb;

    /*
     * Files before directory operands.
     */
    if (a->is_dir_operand != b->is_dir_operand) {
        return a->is_dir_operand ? 1 : -1;
    }

    return compare_body(a, b);
}

/* Sort entries within a directory. */
void sort_entries(FileEntry *entries,
                  size_t count,
                  const LSOptions *opts)
{
    if (count < 2 ||
        opts->sort_type == SORT_NONE) {
        return;
    }

    sort_mode = opts->sort_type;
    time_mode = opts->time_type;
    reverse_order = opts->reverse;

    qsort(entries, count,
          sizeof(*entries), cmp_entries);
}

/* Sort command-line operands. */
void sort_operands(FileEntry *entries,
                   size_t count,
                   const LSOptions *opts)
{
    if (count < 2)
        return;

    if (opts->sort_type == SORT_NONE) {
        /*
         * Stable partition into files and dirs.
         */
        FileEntry *copy = malloc(
            count * sizeof(*copy));

        if (copy == NULL)
            return;

        size_t j = 0;

        for (size_t i = 0; i < count; i++) {
            if (!entries[i].is_dir_operand)
                copy[j++] = entries[i];
        }

        for (size_t i = 0; i < count; i++) {
            if (entries[i].is_dir_operand)
                copy[j++] = entries[i];
        }

        memcpy(entries, copy,
               count * sizeof(*copy));

        free(copy);
        return;
    }

    sort_mode = opts->sort_type;
    time_mode = opts->time_type;
    reverse_order = opts->reverse;

    qsort(entries, count,
          sizeof(*entries), cmp_operands);
}

