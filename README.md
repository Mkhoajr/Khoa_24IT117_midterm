# Midterm Project Report — Implementing `ls(1)` on NetBSD 10.1

**Student:** Cat Minh Khoa  
**Student ID:** 24IT117  
**Course:** System Programming  
**Platform:** NetBSD 10.1 (Oracle VirtualBox, accessed over SSH)  
**Language:** C (C11)  
**Repository:** https://github.com/Mkhoajr/Khoa_24IT117_midterm

## 1. Overview

This project builds **`myls`**, a modular implementation of the Unix `ls(1)` utility, using the NetBSD 10.1 `ls(1)` specification supplied for the midterm. It demonstrates directory traversal, file metadata handling, command-line parsing, sorting, symbolic links, formatted output, and basic error handling in C.

The source is organized into independent `.c` modules and matching header files so that filesystem access, option parsing, sorting, formatting, and utility functions can be maintained separately.

## 2. Implemented Features

The command-line parser accepts the **19 requested options**:

| Option | Implemented behavior |
|---|---|
| `-a`, `-A` | Show all entries, or hidden entries excluding `.` and `..` |
| `-d` | Show directory operands themselves instead of their contents |
| `-F` | Append file-type indicators (such as `/`, `*`, `@`) |
| `-f` | Disable sorting |
| `-i` | Show inode numbers |
| `-l`, `-n` | Long format, with owner/group names or numeric IDs |
| `-h`, `-k`, `-s` | Human-readable sizes, 1 KiB block units, allocated block counts |
| `-R` | Recursively list subdirectories, without following discovered symlinks |
| `-r`, `-S`, `-t` | Reverse, size-based, or time-based sorting |
| `-c`, `-u` | Select status-change or access time |
| `-q`, `-w` | Replace non-printable filename characters, or print raw filenames |

The program also supports multiple path operands, regular files, hidden files, symbolic links, and a nonzero exit status for encountered errors. Options may be combined, e.g., `-lR`.

> **Compatibility note:** These options have been implemented, but the program is a simplified version of NetBSD `ls(1)`. Some edge cases and formatting details are listed under *Known Limitations* rather than claimed as exact replicas of the system utility.

## 3. Project Organization

```text
Khoa_24IT117_midterm/
├── include/
│   ├── directory.h
│   ├── display.h
│   ├── options.h
│   ├── sorting.h
│   └── utils.h
├── src/
│   ├── directory.c
│   ├── display.c
│   ├── main.c
│   ├── options.c
│   ├── sorting.c
│   └── utils.c
├── tests/
│   └── test_ls.sh
├── .gitignore
├── Makefile
└── README.md
```

| Module | Responsibility |
|---|---|
| `main.c` | Entry point, path operands, overall execution |
| `options.c` | Parse command-line flags with `getopt()` |
| `directory.c` | Read directories, allocate entries, recursive traversal |
| `sorting.c` | Sort directory entries and command-line operands |
| `display.c` | Long-format metadata, filenames, links, block totals |
| `utils.c` | Size/block formatting, time selection, path joining |

## 4. Building and Running

**Requirement:** NetBSD 10.1 with a C compiler and `make`.

From the root of the project:

```sh
make clean
make
```

This builds the executable `./myls`. Example commands:

```sh
./myls                          # Current directory
./myls tests/sample             # Specify a directory
./myls -a tests/sample          # Include hidden entries
./myls -lh tests/sample         # Detailed output, readable sizes
./myls -sk tests/sample         # Allocated blocks in 1 KiB units
./myls -lR tests/recursive      # Recursive detailed listing
./myls -F tests/sample          # File-type suffixes
./myls -q tests/special         # Non-printable names as '?'
./myls file1.txt directory1     # Multiple operands
make clean                      # Remove executable and .o files
```

The sample directory names above refer to optional test data; use existing local paths as appropriate.

## 5. Design and System Programming Concepts

- **Filesystem access:** `opendir()`, `readdir()`, `closedir()`, `stat()`, and `lstat()`.
- **Symbolic links:** `readlink()` and `lstat()` distinguish links from their targets during recursion.
- **Metadata:** `struct stat` provides modes, inode values, owners, sizes, blocks, and timestamps; `getpwuid()` and `getgrgid()` resolve account names.
- **Option parsing and ordering:** `getopt()` processes flags; `qsort()` orders entries.
- **Memory management:** Dynamically resized `FileEntry` arrays and allocated paths are released after use.
- **Output:** Long-format widths are calculated from a directory's entries; `-q` and `-w` control non-printable name rendering.

## 6. Testing and Results

The project was **compiled successfully on NetBSD 10.1** using:

```sh
make clean && make
```

The test script was executed with:

```sh
sh tests/test_ls.sh
```

**Observed test result:** **25 PASS, 0 FAIL**. The checks cover startup, major options, recursive traversal, filename policies, multiple operands, basic comparisons with system `ls`, and invalid-path handling.

Additional manual comparisons were performed for `-l`, `-lh`, and `-n` using output redirection and `diff`:

```sh
./myls -l tests/sample > actual.txt
ls -l tests/sample > expected.txt
diff -u expected.txt actual.txt
```

The core metadata and sizes matched the inspected sample. A spacing mismatch in long-format output was identified and the formatter was updated; the final exact-match `diff` after that update should be rerun and recorded before submission. A passing smoke test does **not** by itself establish complete equivalence with NetBSD `ls(1)`.

## 7. Known Limitations

- Human-readable rounding and directory `total` formatting may differ on other filesystems, sparse files, or unusual block-size settings.
- Some interactions among `-f`, time selection, and sorting flags need more comprehensive comparison.
- Special file types, unusual symbolic-link operands, and extremely deep directory trees have not been exhaustively tested.
- Recursion avoids the usual `.`/`..` loops but does not implement general filesystem-cycle detection.
- The test script is a smoke suite, not a complete behavioral conformance suite.

## 8. Repository and Submission

**GitHub repository:** https://github.com/Mkhoajr/Khoa_24IT117_midterm

The repository should contain the source (`src/`, `include/`), `Makefile`, `README.md`, `.gitignore`, and testing scripts. Compiled `myls`, `*.o`, editor backup files, and other generated artifacts are excluded by `.gitignore`.

For submission, include this report (or its PDF export, if required by the e-learning assignment) with the GitHub URL. Confirm that the latest tested source files have been committed and pushed before submitting.

## 9. Conclusion

The project applies core system-programming techniques to recreate the main behavior of the NetBSD `ls(1)` command in a maintainable, modular C program. The current version builds on the target operating system and passes 25 basic tests. Further edge-case comparisons are documented above to make the current level of compatibility transparent.
