#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef __NetBSD__
#include <sys/sysmacros.h>
#endif

#include "display.h"
#include "utils.h"

/*
 * Return the larger of two values.
 */
static size_t max_size(size_t a, size_t b)
{
    return a > b ? a : b;
}

/*
 * Count decimal digits.
 */
static size_t digits(uintmax_t value)
{
    size_t n = 1;

    while (value >= 10) {
        value /= 10;
        n++;
    }

    return n;
}

/*
 * Print filename according to -q and -w.
 *
 * -q: replace non-printable characters with '?'
 * -w: print original filename bytes
 *
 * Default: use -q behavior when stdout is a TTY.
 */
void print_name(const char *name,
                const LSOptions *opts)
{
    int replace =
        opts->name_format == NAME_QUESTION ||
        (opts->name_format == NAME_AUTO &&
         isatty(STDOUT_FILENO));

    if (!replace) {
        fputs(name, stdout);
        return;
    }

    mbstate_t state;
    memset(&state, 0, sizeof(state));

    const char *p = name;
    size_t remaining = strlen(name);

    while (remaining > 0) {
        wchar_t wc;

        size_t len = mbrtowc(
            &wc, p, remaining, &state);

        if (len == (size_t)-1 ||
            len == (size_t)-2) {
            putchar('?');

            p++;
            remaining--;

            memset(&state, 0, sizeof(state));
            continue;
        }

        if (len == 0)
            break;

        if (iswprint(wc)) {
            fwrite(p, 1, len, stdout);
        } else {
            putchar('?');
        }

        p += len;
        remaining -= len;
    }
}

/*
 * Return classification suffix for -F.
 */
static char suffix(mode_t mode)
{
    if (S_ISDIR(mode))
        return '/';

    if (S_ISLNK(mode))
        return '@';

    if (S_ISSOCK(mode))
        return '=';

    if (S_ISFIFO(mode))
        return '|';

#ifdef S_ISWHT
    if (S_ISWHT(mode))
        return '%';
#endif

    if (S_ISREG(mode) &&
        (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return '*';

    return '\0';
}

/*
 * Convert file mode to the ls permission string.
 */
static void mode_string(mode_t mode, char out[12])
{
#ifdef __NetBSD__
    strmode(mode, out);
#else
    memset(out, '-', 10);

    out[10] = ' ';
    out[11] = '\0';

    if (S_ISDIR(mode))
        out[0] = 'd';
    else if (S_ISLNK(mode))
        out[0] = 'l';
    else if (S_ISFIFO(mode))
        out[0] = 'p';
    else if (S_ISSOCK(mode))
        out[0] = 's';
    else if (S_ISCHR(mode))
        out[0] = 'c';
    else if (S_ISBLK(mode))
        out[0] = 'b';

    const mode_t masks[] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };

    const char chars[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++) {
        if (mode & masks[i])
            out[i + 1] = chars[i];
    }

    if (mode & S_ISUID) {
        out[3] = (mode & S_IXUSR) ? 's' : 'S';
    }

    if (mode & S_ISGID) {
        out[6] = (mode & S_IXGRP) ? 's' : 'S';
    }

    if (mode & S_ISVTX) {
        out[9] = (mode & S_IXOTH) ? 't' : 'T';
    }
#endif
}

/*
 * Determine owner and group strings.
 *
 * -l: symbolic names
 * -n: numeric UID and GID
 */
static void owner_name(const struct stat *st,
                       const LSOptions *opts,
                       char owner[128],
                       char group[128])
{
    snprintf(owner, 128, "%ju",
             (uintmax_t)st->st_uid);

    snprintf(group, 128, "%ju",
             (uintmax_t)st->st_gid);

    if (opts->format == FORMAT_LONG) {
        struct passwd *pw = getpwuid(st->st_uid);
        struct group *gr = getgrgid(st->st_gid);

        if (pw != NULL) {
            snprintf(owner, 128, "%s", pw->pw_name);
        }

        if (gr != NULL) {
            snprintf(group, 128, "%s", gr->gr_name);
        }
    }
}

/*
 * Format file size or device major/minor.
 */
static void size_string(const struct stat *st,
                        const LSOptions *opts,
                        char buf[64])
{
    if (S_ISCHR(st->st_mode) ||
        S_ISBLK(st->st_mode)) {
        snprintf(buf, 64, "%u, %u",
                 (unsigned)major(st->st_rdev),
                 (unsigned)minor(st->st_rdev));
    } else if (opts->size_type == SIZE_HUMAN &&
               st->st_size >= 0) {
        format_human(
            (uintmax_t)st->st_size,
            buf,
            64);
    } else {
        snprintf(buf, 64, "%jd",
                 (intmax_t)st->st_size);
    }
}

/*
 * Format the selected file timestamp.
 */
static void time_string(const struct stat *st,
                        const LSOptions *opts,
                        char buf[64])
{
    time_t selected =
        selected_time(st, opts->time_type);

    time_t now = time(NULL);

    struct tm *tm_info = localtime(&selected);

    if (tm_info == NULL) {
        snprintf(buf, 64, "?");
        return;
    }

    /*
     * Old or future timestamps display a year.
     */
    const char *fmt =
        (selected > now ||
         (now >= selected &&
          now - selected > 15552000))
        ? "%b %e  %Y"
        : "%b %e %H:%M";

    if (strftime(buf, 64, fmt, tm_info) == 0) {
        snprintf(buf, 64, "?");
    }
}

/*
 * Calculate maximum column widths.
 *
 * This is needed to align long-format output.
 */
void calculate_widths(const FileEntry *entries,
                      size_t count,
                      const LSOptions *opts,
                      DisplayWidths *w)
{
    memset(w, 0, sizeof(*w));

    for (size_t i = 0; i < count; i++) {
        const struct stat *st = &entries[i].st;

        char owner[128];
        char group[128];
        char sz[64];
        char blk[64];

        w->inode = max_size(
            w->inode,
            digits((uintmax_t)st->st_ino));

        if (st->st_blocks >= 0) {
            format_blocks(
                (uintmax_t)st->st_blocks,
                opts,
                blk,
                sizeof(blk));

            w->blocks = max_size(
                w->blocks,
                strlen(blk));
        }

        if (opts->format == FORMAT_SHORT)
            continue;

        owner_name(st, opts, owner, group);
        size_string(st, opts, sz);

        w->links = max_size(
            w->links,
            digits((uintmax_t)st->st_nlink));

        w->owner = max_size(
            w->owner,
            strlen(owner));

        w->group = max_size(
            w->group,
            strlen(group));

        w->size = max_size(
            w->size,
            strlen(sz));
    }
}

/*
 * Print symbolic link destination.
 */
static int link_target(const FileEntry *entry,
                       const LSOptions *opts)
{
    size_t capacity = 128;

    for (;;) {
        char *buf = malloc(capacity + 1);

        if (buf == NULL) {
            perror("malloc");
            return 1;
        }

        ssize_t length = readlink(
            entry->path,
            buf,
            capacity);

        if (length < 0) {
            perror(entry->path);
            free(buf);
            return 1;
        }

        if ((size_t)length < capacity) {
            buf[length] = '\0';

            fputs(" -> ", stdout);
            print_name(buf, opts);

            free(buf);
            return 0;
        }

        free(buf);

        if (capacity > (SIZE_MAX - 1) / 2) {
            fprintf(stderr,
                    "myls: symbolic link too long\n");
            return 1;
        }

        capacity *= 2;
    }
}

/*
 * Print a single filesystem entry.
 */
int print_entry(const FileEntry *entry,
                const LSOptions *opts,
                const DisplayWidths *w)
{
    const struct stat *st = &entry->st;
    int status = 0;

    /*
     * -i: inode number.
     */
    if (opts->show_inode) {
        printf("%*ju ",
               (int)w->inode,
               (uintmax_t)st->st_ino);
    }

    /*
     * -s: allocated filesystem blocks.
     */
    if (opts->show_blocks) {
        char blk[64];

        format_blocks(
            st->st_blocks < 0
                ? 0
                : (uintmax_t)st->st_blocks,
            opts,
            blk,
            sizeof(blk));

        printf("%*s ",
               (int)w->blocks,
               blk);
    }

    /*
     * -l and -n: long-format metadata.
     */
    if (opts->format != FORMAT_SHORT) {
        char mode[12];
        char owner[128];
        char group[128];
        char sz[64];
        char when[64];

        mode_string(st->st_mode, mode);
        owner_name(st, opts, owner, group);
        size_string(st, opts, sz);
        time_string(st, opts, when);

        /*
         * Stage 6: align with NetBSD ls.
         *
         * Two spaces after permission string.
         * Two spaces between group and size field.
         */
        printf("%.10s  %*ju %-*s  %-*s  %*s %s ",
               mode,
               (int)w->links,
               (uintmax_t)st->st_nlink,
               (int)w->owner,
               owner,
               (int)w->group,
               group,
               (int)w->size,
               sz,
               when);
    }

    /*
     * Filename.
     */
    print_name(entry->name, opts);

    /*
     * -F: file classification suffix.
     */
    if (opts->classify) {
        char s = suffix(st->st_mode);

        if (s != '\0')
            putchar(s);
    }

    /*
     * Long-format symbolic link destination.
     */
    if (opts->format != FORMAT_SHORT &&
        S_ISLNK(st->st_mode)) {
        status = link_target(entry, opts);
    }

    putchar('\n');

    return status;
}

/*
 * Print directory total.
 *
 * Keeps the Stage 6 total formatting currently
 * verified against tests/sample.
 */
void print_total(const FileEntry *entries,
                 size_t count,
                 const LSOptions *opts)
{
    uintmax_t sum_blocks = 0;
    uintmax_t sum_sizes = 0;

    char buf[64];

    for (size_t i = 0; i < count; i++) {
        const struct stat *st = &entries[i].st;

        if (st->st_blocks > 0 &&
            sum_blocks <=
                UINTMAX_MAX -
                (uintmax_t)st->st_blocks) {
            sum_blocks +=
                (uintmax_t)st->st_blocks;
        }

        if (st->st_size > 0 &&
            sum_sizes <=
                UINTMAX_MAX -
                (uintmax_t)st->st_size) {
            sum_sizes +=
                (uintmax_t)st->st_size;
        }
    }

    /*
     * Human-readable long-format total.
     * This preserves the tested Stage 4/5 result.
     */
    if (opts->size_type == SIZE_HUMAN &&
        opts->format != FORMAT_SHORT &&
        !opts->show_blocks) {
        format_human(
            sum_sizes,
            buf,
            sizeof(buf));
    } else {
        format_blocks(
            sum_blocks,
            opts,
            buf,
            sizeof(buf));
    }

    printf("total %s\n", buf);
}

