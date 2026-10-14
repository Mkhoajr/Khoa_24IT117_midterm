#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/stat.h>

#include "options.h"
#include "sorting.h"
#include "directory.h"
#include "display.h"

/*
 * Program entry point.
 */
int main(int argc, char *argv[])
{
    LSOptions opts;
    FileEntry *operands;

    size_t count = 0;
    size_t files = 0;
    size_t dirs = 0;

    int start;
    int status = 0;

    setlocale(LC_CTYPE, "");

    start = parse_options(argc, argv, &opts);

    if (start < 0)
        return 1;

    /*
     * No operands: list current directory.
     */
    if (start == argc) {
        return list_directory(
            ".", &opts, opts.recursive);
    }

    /*
     * Allocate storage for all path operands.
     */
    operands = calloc(
        (size_t)(argc - start),
        sizeof(*operands)
    );

    if (operands == NULL) {
        perror("calloc");
        return 1;
    }

    /*
     * Inspect every operand.
     */
    for (int i = start; i < argc; i++) {
        struct stat st;

        /*
         * Follow symlink operands in the basic
         * short-output case, unless -d or -F.
         */
        int follow =
            !opts.directory_only &&
            !opts.classify &&
            opts.format == FORMAT_SHORT;

        int result;

        if (follow)
            result = stat(argv[i], &st);
        else
            result = lstat(argv[i], &st);

        if (result != 0) {
            /*
             * Broken symbolic links can still
             * be displayed using lstat().
             */
            if (!follow ||
                lstat(argv[i], &st) != 0) {
                perror(argv[i]);
                status = 1;
                continue;
            }
        }

        operands[count].name = strdup(argv[i]);
        operands[count].path = strdup(argv[i]);

        if (operands[count].name == NULL ||
            operands[count].path == NULL) {
            perror("strdup");

            free(operands[count].name);
            free(operands[count].path);

            free_entries(operands, count);
            return 1;
        }

        operands[count].st = st;

        operands[count].is_dir_operand =
            S_ISDIR(st.st_mode) &&
            !opts.directory_only;

        if (operands[count].is_dir_operand)
            dirs++;
        else
            files++;

        count++;
    }

    /*
     * Sort files and directory operands.
     * Directory operands are placed after files.
     */
    sort_operands(operands, count, &opts);

    /*
     * Print non-directory operands together.
     */
    DisplayWidths widths;

    calculate_widths(
        operands, files, &opts, &widths);

    for (size_t i = 0; i < files; i++) {
        if (print_entry(
                &operands[i],
                &opts,
                &widths) != 0) {
            status = 1;
        }
    }

    /*
     * List directory operands.
     */
    for (size_t i = files; i < count; i++) {
        if (i > files || files > 0)
            putchar('\n');

        if (list_directory(
                operands[i].path,
                &opts,
                dirs > 1 ||
                files > 0 ||
                opts.recursive) != 0) {
            status = 1;
        }
    }

    free_entries(operands, count);

    return status;
}

