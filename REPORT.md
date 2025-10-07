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
  

