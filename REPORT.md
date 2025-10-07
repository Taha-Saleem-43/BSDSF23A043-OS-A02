## REPORT.md

### 1. Difference between `stat()` and `lstat()`

The `stat()` and `lstat()` system calls are both used to retrieve information about a file, stored in a `struct stat` structure.

- **`stat()`** follows symbolic links and returns information about the *target file* that the link points to.  
- **`lstat()`** does **not** follow symbolic links and returns information about the *link itself*.

**In the context of the `ls` command**, it is more appropriate to use **`lstat()`** because it allows the program to detect and display symbolic links accurately (e.g., showing `linkfile -> targetfile`).  
If `stat()` were used instead, symbolic links would appear as regular files since it would follow the link to its target.

---

### 2. Extracting file type and permissions using `st_mode`

The `st_mode` field in `struct stat` is an integer that encodes both:
- the **file type** (regular file, directory, symlink, etc.), and  
- the **permission bits** (read, write, execute for owner, group, others).

We can extract these using **bitwise operators** (`&`) along with predefined **macros** from `<sys/stat.h>`.

#### (a) Checking file type
```c
if (S_ISDIR(st.st_mode))  printf("This is a directory\n");
if (S_ISREG(st.st_mode))  printf("This is a regular file\n");
if (S_ISLNK(st.st_mode))  printf("This is a symbolic link\n");


## Report for Feature-3: Column Display (v1.2.0)

### 1. Logic for "Down Then Across" Columnar Format

In the "down then across" layout, filenames are first collected into an array and then printed row by row instead of sequentially in a single loop. The basic idea is:

- All filenames are read and stored first.
- The maximum filename length is determined to calculate proper column width.
- Using the terminal width (from `ioctl`), the program determines how many columns can fit.
- The total number of rows is then computed using the total number of files and the number of columns.
- The printing logic iterates over each row, printing filenames at positions:



This approach ensures that the output is aligned in columns and fills down each column before moving to the next one across.  

A simple single loop through the filenames is **insufficient** because it would print the files only in a single column (top to bottom), without controlling the placement into multiple columns. To achieve a grid-like display that adapts to terminal width, you need to structure output by rows and columns rather than just one sequence.

---

### 2. Purpose of the `ioctl` System Call

The `ioctl` system call with the `TIOCGWINSZ` argument is used to **query the terminal size**, specifically to obtain the current width (number of columns).  
This allows the program to **dynamically adjust** the number of columns displayed based on the user's terminal width.

**If only a fixed width (e.g., 80 columns) were used:**
- The layout would not adapt when the terminal is resized.
- On wider screens, the output would waste space (too few columns).
- On narrower screens, lines might wrap incorrectly, breaking the alignment.

Using `ioctl` ensures the output always fits neatly in the visible area of the terminal.

---

### Example Summary

| Concept | Description |
|----------|--------------|
| **Down-then-across layout** | Prints filenames vertically down each column, then moves across to next column. |
| **Why not single loop** | A single loop can't compute column/row positions, only lists sequentially. |
| **`ioctl` purpose** | Detects actual terminal width dynamically. |
| **Fixed width issue** | Output misaligns or wastes space on different terminal sizes. |

# Feature 4: Horizontal Column Display (-x) – Report

## 1. Implementation Complexity: Vertical vs Horizontal Display

- **Vertical ("down then across") layout (`-c` / `do_ls_columns`)**  
  - Files are printed **column by column** (top to bottom, then left to right).  
  - Requires **pre-calculation of number of rows** and **column widths** before printing.  
  - Steps:
    1. Count files and find the **longest filename** for column width.  
    2. Compute **number of columns** and **number of rows** based on terminal width.  
    3. During printing, calculate **index = c * rows + r** to fetch the correct filename.  
  - **Complexity:** Higher, because you must know the **rows and columns in advance** to avoid misalignment.

- **Horizontal ("across") layout (`-x` / `do_ls_horizontal`)**  
  - Files are printed **left to right**, wrapping to next line as needed.  
  - Only needs **maximum filename width** and **current horizontal position** on the screen.  
  - Steps:
    1. Count files and find **longest filename**.  
    2. Loop through files, print each, and check if **next filename exceeds terminal width**.  
    3. Wrap line if necessary.  
  - **Complexity:** Lower, because you do **not need row/column pre-calculation**, only a running counter.

**Conclusion:**  
> The vertical layout (`-c`) requires more pre-calculation than the horizontal layout (`-x`) because it needs to compute both **rows and columns** before printing, whereas horizontal printing can proceed **sequentially with a simple width check**.

---

## 2. Display Mode Management Strategy

- Introduced **flag variables** in `main()`:
  ```c
  int longflag = 0;      // -l
  int columnflag = 0;    // 0 = default, 1 = vertical, 2 = horizontal


# Feature 5: Alphabetical Sort (ls-v1.4.0) Report

## 1. Why is it necessary to read all directory entries into memory before you can sort them?

To sort files alphabetically, the program must have access to **all filenames at once**. The `readdir()` function reads one entry at a time, so without storing them, sorting is impossible.

**Potential drawbacks for directories with millions of files:**

---

## 2. Purpose and signature of the comparison function for `qsort()`

`qsort()` is a generic C library function that needs a **comparison function** to determine the order of elements.

**Signature:**

```c
int compare_fileent(const void *a, const void *b);




  

