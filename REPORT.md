# LS Command Features Report

```text
==============================
1. Difference Between stat() and lstat()
==============================

stat():   Follows symbolic links; returns info about the TARGET file.
lstat():  Does NOT follow symbolic links; returns info about the LINK itself.

In ls: Use lstat() to correctly detect symbolic links.
Using stat() would make links appear as regular files.

==============================
2. Extracting File Type and Permissions (st_mode)
==============================

// File type check
if (S_ISDIR(st.st_mode))  printf("Directory\n");
if (S_ISREG(st.st_mode))  printf("Regular File\n");
if (S_ISLNK(st.st_mode))  printf("Symbolic Link\n");

// Executable permission check
if (st.st_mode & S_IXUSR) printf("Owner executable\n");
if (st.st_mode & S_IXGRP) printf("Group executable\n");
if (st.st_mode & S_IXOTH) printf("Others executable\n");

==============================
3. Column Display (Feature-3, v1.2.0)
==============================

Down-then-Across Layout Logic:

1. Read all filenames into an array.
2. Find maximum filename length.
3. Get terminal width using ioctl(TIOCGWINSZ).
4. Compute number of columns and rows.
5. Print files row by row:
   index = column * rows + row

Single-loop printing cannot organize files into proper columns.

Purpose of ioctl:

- Dynamically fetch terminal width.
- Allows proper column alignment.
- Prevents wrapping issues.

Concepts Table:

Layout              : Prints vertically down then across
Single loop issue   : Cannot calculate row/column positions
ioctl               : Detects terminal width dynamically
Fixed width issue   : Misalignment or wasted space

==============================
4. Horizontal Display (Feature-4, -x)
==============================

Aspect            Vertical (-c)                     Horizontal (-x)
---------------------------------------------------------------------
Print order       Column by column                  Left to right
Complexity        Pre-calculate rows & columns     Simple sequential with width check
Steps             Compute max name, rows, cols;   Print each file, wrap line if exceeds width
                  use index = c*rows + r

==============================
5. Alphabetical Sort (Feature-5, v1.4.0)
==============================

- Must read all entries into memory for sorting.
- readdir() alone cannot sort; it reads sequentially.

qsort() Comparison Function:

int compare_fileent(const void *a, const void *b);

==============================
6. ANSI Color Codes
==============================

Escape sequence format: \033[<attributes>;<fg>;<bg>m

\033         -> escape character
<attributes> -> 0=reset, 1=bold
<fg>         -> 30-37 (foreground colors)
<bg>         -> 40-47 (background colors)

Example: Green text
printf("\033[0;32mHello, World!\033[0m\n");

==============================
7. Recursive ls Concepts
==============================

Base Case:
- Stops recursion when no subdirectories remain (or only '.' and '..').
- Prevents infinite recursion and stack overflow.

Full Path Importance:
- Always construct full path: "parent/subdir"
- Ensures recursion points to the correct filesystem location.

Example:
do_ls("parent/subdir");

==============================
8. Summary
==============================

- lstat() detects symbolic links correctly.
- st_mode is used for file type and permissions.
- Down-then-across layout uses pre-calculated rows & columns.
- Horizontal layout (-x) is simpler; prints left-to-right.
- ANSI codes color text; executable files are determined via S_IXUSR, S_IXGRP, S_IXOTH.
- Recursive ls needs a base case and full paths to work correctly.
