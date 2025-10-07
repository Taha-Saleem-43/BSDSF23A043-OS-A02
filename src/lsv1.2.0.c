/*
 * ls-v1.1.0 : adds -l long listing format
 * Usage:
 *   ./bin/lsv1.0.0         # simple listing
 *   ./bin/lsv1.0.0 -l      # long listing of current directory
 *   ./bin/lsv1.0.0 -l /etc /usr
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>
#include <sys/ioctl.h>

extern int errno;

void do_ls(const char *dir);
void do_ls_long(const char *dir);
void mode_to_perm(mode_t mode, char *out);
char filetype_char(mode_t mode);
void do_ls_columns(const char *dir);


/* small helper to join dir + "/" + name into path buffer safely */
static void joinpath(const char *dir, const char *name, char *out, size_t outlen) {
    if (snprintf(out, outlen, "%s/%s", dir, name) >= (int)outlen) {
        /* fallback truncation */
        out[outlen-1] = '\0';
    }
}

int compare_names(const void *a, const void *b) {
    const char *const *pa = a;
    const char *const *pb = b;
    return strcmp(*pa, *pb);
}


int main(int argc, char const *argv[])
{
    int opt;
    int longflag = 0;
    int columnflag = 0;

    /* parse -l (long listing) and -c (column display) options */
    while ((opt = getopt(argc, (char * const *)argv, "lc")) != -1) {
        if (opt == 'l') longflag = 1;
        if (opt == 'c') columnflag = 1;
    }

    /* if no arguments, list current directory */
    if (optind == argc) {
        if (longflag)
            do_ls_long(".");
        else if (columnflag)
            do_ls_columns(".");
        else
            do_ls_columns(".");   // Default behavior: column display (Feature 3)
    } 
    else {
        /* multiple directories specified */
        for (int i = optind; i < argc; i++) {
            if (argc - optind > 1)
                printf("%s:\n", argv[i]);

            if (longflag)
                do_ls_long(argv[i]);
            else if (columnflag)
                do_ls_columns(argv[i]);
            else
                do_ls_columns(argv[i]);  // Default behavior: column display

            if (i + 1 < argc)
                puts("");
        }
    }

    return 0;
}


void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
    {
        perror("readdir failed");
    }

    closedir(dp);
}

/* Data holder for pass-through */
typedef struct {
    char *name;
    struct stat st;
} fileent;

void do_ls_long(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }

    /* dynamic array of file entries */
    fileent *list = NULL;
    size_t count = 0, cap = 0;

    size_t max_links = 0, max_user = 0, max_group = 0, max_size = 0;

    errno = 0;
    /* first pass: collect stats and compute column widths */
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        /* ensure capacity */
        if (count == cap) {
            cap = cap ? cap * 2 : 64;
            fileent *tmp = realloc(list, cap * sizeof(fileent));
            if (!tmp) { perror("realloc"); closedir(dp); free(list); return; }
            list = tmp;
        }

        /* copy name */
        list[count].name = strdup(entry->d_name);
        if (!list[count].name) { perror("strdup"); closedir(dp); /* free */ for (size_t j=0;j<count;j++) free(list[j].name); free(list); return; }

		/* build path */
		char path[PATH_MAX];
		joinpath(dir, entry->d_name, path, sizeof(path));
	
		/* use lstat to preserve symlink info */
		if (lstat(path, &list[count].st) == -1) {   // ? FIXED
	    /* if error, set zeros and continue */
	    perror(path);
	    memset(&list[count].st, 0, sizeof(struct stat));
		}
	
		/* compute widths */
		/* link count width */
		{
	    char buf[32];
	    int n = snprintf(buf, sizeof(buf), "%lu", (unsigned long)list[count].st.st_nlink);
	    if (n > (int)max_links) max_links = n;
		}

        /* user name width */
        {
            struct passwd *pw = getpwuid(list[count].st.st_uid);
            size_t len = pw ? strlen(pw->pw_name) : snprintf(NULL, 0, "%u", list[count].st.st_uid);
            if (len > max_user) max_user = len;
        }
        /* group name width */
        {
            struct group *gr = getgrgid(list[count].st.st_gid);
            size_t len = gr ? strlen(gr->gr_name) : snprintf(NULL, 0, "%u", list[count].st.st_gid);
            if (len > max_group) max_group = len;
        }
        /* size width */
        {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "%lld", (long long)list[count].st.st_size);
            if (n > (int)max_size) max_size = n;
        }

        count++;
    }

    if (errno != 0) {
        perror("readdir");
        /* continue to print what we have */
    }

    /* second pass: print entries using computed widths */
    for (size_t i = 0; i < count; ++i) {
        char perms[12] = "----------";
        mode_to_perm(list[i].st.st_mode, perms);
        char ftype = filetype_char(list[i].st.st_mode);

        /* links */
        char links_buf[64];
        snprintf(links_buf, sizeof(links_buf), "%*lu", (int)max_links, (unsigned long)list[i].st.st_nlink);

        /* owner */
        struct passwd *pw = getpwuid(list[i].st.st_uid);
        char owner_buf[128];
        if (pw) snprintf(owner_buf, sizeof(owner_buf), "%-*s", (int)max_user, pw->pw_name);
        else snprintf(owner_buf, sizeof(owner_buf), "%-*u", (int)max_user, list[i].st.st_uid);

        /* group */
        struct group *gr = getgrgid(list[i].st.st_gid);
        char group_buf[128];
        if (gr) snprintf(group_buf, sizeof(group_buf), "%-*s", (int)max_group, gr->gr_name);
        else snprintf(group_buf, sizeof(group_buf), "%-*u", (int)max_group, list[i].st.st_gid);

        /* size */
        char size_buf[64];
        snprintf(size_buf, sizeof(size_buf), "%*lld", (int)max_size, (long long)list[i].st.st_size);

        /* time: format similar to ls -l: "Mon _d HH:MM" or "Mon _d  YYYY" depending on age.
           For simplicity we will print "Mon _d HH:MM" */
        char timebuf[64];
        struct tm *tm = localtime(&list[i].st.st_mtime);
        if (tm) {
            strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
        } else {
            strncpy(timebuf, "??? ?? ??:??", sizeof(timebuf));
            timebuf[sizeof(timebuf)-1] = '\0';
        }

        /* filename and symlink target if any */
        char path[PATH_MAX];
        joinpath(dir, list[i].name, path, sizeof(path));

        if (S_ISLNK(list[i].st.st_mode)) {
            /* readlink target */
            char target[PATH_MAX+1];
            ssize_t tlen = readlink(path, target, PATH_MAX);
            if (tlen != -1) {
                target[tlen] = '\0';
                printf("%c%s %s %s %s %s %s %s -> %s\n",
                       ftype, perms+1, /* perms string is 10 chars like -rwxr-xr-x; we keep ftype separately */
                       links_buf,
                       owner_buf,
                       group_buf,
                       size_buf,
                       timebuf,
                       list[i].name,
                       target);
            } else {
                printf("%c%s %s %s %s %s %s %s\n",
                       ftype, perms+1,
                       links_buf, owner_buf, group_buf, size_buf, timebuf, list[i].name);
            }
        } else {
            printf("%c%s %s %s %s %s %s %s\n",
                   ftype, perms+1,
                   links_buf, owner_buf, group_buf, size_buf, timebuf, list[i].name);
        }

        free(list[i].name);
    }

    free(list);
    closedir(dp);
}

void do_ls_columns(const char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct winsize w;
    int term_width = 80; // default fallback if ioctl fails

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    dp = opendir(dir);
    if (dp == NULL)
    {
        perror("Cannot open directory");
        return;
    }

    // Step 1: Collect file names
    char **names = NULL;
    int count = 0;
    size_t max_len = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue; // skip hidden files

        names = realloc(names, sizeof(char *) * (count + 1));
        names[count] = strdup(entry->d_name);

        size_t len = strlen(entry->d_name);
        if (len > max_len)
            max_len = len;

        count++;
    }
    closedir(dp);

    if (count == 0)
        return;

    // Step 2: Sort names alphabetically
    qsort(names, count, sizeof(char *), compare_names);

    // Step 3: Calculate column layout
    int spacing = 2;
    int col_width = max_len + spacing;
    int cols = term_width / col_width;
    if (cols < 1)
        cols = 1;
    int rows = (count + cols - 1) / cols;

    // Step 4: Print down-then-across
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int index = c * rows + r;
            if (index < count)
                printf("%-*s", col_width, names[index]);
        }
        putchar('\n');
    }

    // Step 5: Cleanup
    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

/* Convert st_mode to permission string in "rwx" form and special bits.
   out should be at least 11 bytes (including leading filetype char if you want,
   but this function sets 10 chars like "drwxr-xr-x" except we return only perms)
*/
void mode_to_perm(mode_t mode, char *out) {
    /* we will fill out[0..9] as r/w/x triplets; caller may add filetype separately */
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');

    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');

    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');

    out[9] = '\0';
}

/* return filetype character like ls: '-', 'd', 'l', 'c', 'b', 'p', 's' */
char filetype_char(mode_t mode) {
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '?';
}



