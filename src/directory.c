#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "directory.h"
#include "display.h"
#include "utils.h"

/* Determine whether an entry is visible. */
static int should_show(const char *name,
                       const LSOptions *opts)
{
    if (opts->show_all)
        return 1;

    if (opts->almost_all) {
        return strcmp(name, ".") != 0 &&
               strcmp(name, "..") != 0;
    }

    return name[0] != '.';
}

/* Free all allocated directory entries. */
void free_entries(FileEntry *entries,
                  size_t count)
{
    for (size_t i = 0; i < count; i++) {
        free(entries[i].name);
        free(entries[i].path);
    }

    free(entries);
}

/*
 * Read directory entries into a dynamic array.
 */
int read_directory(const char *path,
                   const LSOptions *opts,
                   FileEntry **out,
                   size_t *out_count)
{
    DIR *dir = opendir(path);

    FileEntry *entries = NULL;

    size_t count = 0;
    size_t capacity = 0;

    int status = 0;

    struct dirent *de;

    *out = NULL;
    *out_count = 0;

    if (dir == NULL) {
        perror(path);
        return 1;
    }

    while (1) {
        char *name;
        char *full;
        struct stat st;

        errno = 0;

        de = readdir(dir);

        if (de == NULL) {
            if (errno != 0) {
                perror("readdir");
                status = 1;
            }

            break;
        }

        if (!should_show(de->d_name, opts))
            continue;

        full = join_path(path, de->d_name);

        if (full == NULL) {
            perror("join_path");
            status = 1;
            break;
        }

        if (lstat(full, &st) != 0) {
            perror(full);
            free(full);
            status = 1;
            continue;
        }

        name = strdup(de->d_name);

        if (name == NULL) {
            perror("strdup");
            free(full);
            status = 1;
            break;
        }

        /*
         * Grow the dynamic array as necessary.
         */
        if (count == capacity) {
            size_t next =
                capacity ? capacity * 2 : 16;

            FileEntry *tmp;

            if (next < capacity ||
                next > SIZE_MAX / sizeof(*entries)) {
                fprintf(stderr,
                        "myls: too many entries\n");

                free(name);
                free(full);
                status = 1;
                break;
            }

            tmp = realloc(
                entries,
                next * sizeof(*entries));

            if (tmp == NULL) {
                perror("realloc");

                free(name);
                free(full);
                status = 1;
                break;
            }

            entries = tmp;
            capacity = next;
        }

        entries[count].name = name;
        entries[count].path = full;
        entries[count].st = st;
        entries[count].is_dir_operand = 0;

        count++;
    }

    if (closedir(dir) != 0) {
        perror("closedir");
        status = 1;
    }

    *out = entries;
    *out_count = count;

    return status;
}

/*
 * List directory contents and optionally recurse.
 */
int list_directory(const char *path,
                   const LSOptions *opts,
                   int header)
{
    FileEntry *entries = NULL;
    size_t count = 0;

    int status = read_directory(
        path, opts, &entries, &count);

    DisplayWidths widths;

    /*
     * Stop if directory cannot be read.
     */
    if (status &&
        entries == NULL &&
        count == 0) {
        return status;
    }

    sort_entries(entries, count, opts);

    /*
     * Print directory heading when appropriate.
     */
    if (header) {
        print_name(path, opts);
        puts(":");
    }

    /*
     * Display total block count.
     */
    if (opts->format != FORMAT_SHORT ||
        (opts->show_blocks &&
         isatty(STDOUT_FILENO))) {
        print_total(entries, count, opts);
    }

    /*
     * Calculate field widths for aligned output.
     */
    calculate_widths(
        entries, count, opts, &widths);

    /*
     * Print the directory entries.
     */
    for (size_t i = 0; i < count; i++) {
        if (print_entry(
                &entries[i],
                opts,
                &widths) != 0) {
            status = 1;
        }
    }

    /*
     * Visit subdirectories recursively.
     */
    if (opts->recursive) {
        for (size_t i = 0; i < count; i++) {
            const FileEntry *e = &entries[i];

            if (strcmp(e->name, ".") == 0 ||
                strcmp(e->name, "..") == 0) {
                continue;
            }

            /*
             * Do not follow symbolic links.
             */
            if (!S_ISDIR(e->st.st_mode))
                continue;

            putchar('\n');

            if (list_directory(
                    e->path, opts, 1) != 0) {
                status = 1;
            }
        }
    }

    free_entries(entries, count);

    return status;
}

