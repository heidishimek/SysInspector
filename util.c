#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <math.h>

#include "util.h"
#include "logger.h"

int proc_open(char *proc_dir, char *file_path, int fds[2])
{
    int dir_fd = open(proc_dir, O_RDONLY);
    if (dir_fd == -1) {
        return -1;
    }

    int file_fd = openat(dir_fd, file_path, O_RDONLY);
    if (file_fd == -1) {
        close(dir_fd);
        return -1;
    }

    fds[0] = dir_fd;
    fds[1] = file_fd;

    return 0;
}

int proc_close(int fds[2])
{
    int res1 = close(fds[0]);
    int res2 = close(fds[1]);

    if (res1 != 0 || res2 != 0) {
        return -1;
    }

    return 0;
}

ssize_t lineread(int fd, char *buf, size_t sz)
{
    size_t total_read = 0;
    while (total_read < sz) {
        char c;
        ssize_t read_sz = read(fd, &c, 1);
        if (read_sz == 0) {
            return total_read;
        } else if (read_sz == -1) {
            return -1;
        }

        buf[total_read] = c;
        total_read += read_sz;
        if (c == '\n') {
            return total_read;
        }
    }
    return total_read;
}

char *dynamic_lineread(int fd)
{
    size_t buf_sz = 64;
    char *buf = NULL;
    size_t read_total = 0;

    while (true) {
        char *tmp_buf = realloc(buf, buf_sz);
        if (tmp_buf == NULL) {
            free(buf);
            return NULL;
        }

        buf = tmp_buf;

        ssize_t read_sz = lineread(fd, buf + read_total, buf_sz - read_total);

        if (read_sz == 0 || read_sz == -1) {
            free(buf);
            return NULL;
        }

        read_total += read_sz;
        size_t last_character = read_total - 1;
        if (buf[last_character] == '\n' || read_total < buf_sz) {
            buf[last_character] = '\0';
            return buf;
        }

        buf_sz = buf_sz * 2;
    }

    return NULL;
}

ssize_t one_lineread(int fd, char *buf, size_t sz)
{
    size_t total_read = 0;
  
    char c;
    ssize_t read_sz = read(fd, &c, 1);
    if (read_sz == -1) {
        return -1;
    }

    buf[total_read] = c;
    total_read += read_sz;
    if (c == '\0') {
        return total_read;
    }
    
    return total_read;

}

char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

void display_percbar(char *buf, double frac)
{
    double percent = frac * 100;
    if (percent <= 0.00000 || isnan(percent)) {
        percent = 0;
        sprintf(buf, "[--------------------] %.1f%%", percent);
    } else if (percent >= 100) {
        percent = 100;
        sprintf(buf, "[####################] %.1f%%", percent);
    } else {
        int scale = ((round(percent) / 5) + 1);
        char bar[22];
        bar[0] = '[';
        bar[21] = ']';
        int i, j;
        for (i = 1; i < scale; i++) {
            bar[i] = '#';
        }
        for (j = i; j < 21; j++) {
            bar[j] = '-';
        }
        sprintf(buf,"%s %.1f%%", bar, percent);
    }
}

void human_readable_size(char *buf, size_t buf_sz,
        double size, unsigned int decimals)
{
    char *units[] = {"KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};
    int i = 0;
    while (size >= 1024) {
        size = size / 1024;
        i++;
    }

    if (decimals == 1) {
        sprintf(buf, "%.1f %s", size, units[i]);
    } else if (decimals == 2) {
        sprintf(buf, "%.2f %s", size, units[i]);
    } else if (decimals == 3) {
        sprintf(buf, "%.3f %s", size, units[i]);
    } else if (decimals == 4) {
        sprintf(buf, "%.4f %s", size, units[i]);
    } else if (decimals == 5) {
        sprintf(buf, "%.5f %s", size, units[i]);
    } else if (decimals == 6) {
        sprintf(buf, "%.6f %s", size, units[i]);
    } else if (decimals == 7) {
        sprintf(buf, "%.7f %s", size, units[i]);
    } else if (decimals == 8) {
        sprintf(buf, "%.8f %s", size, units[i]);
    } else {
        sprintf(buf, "%.*f %s", i, size, units[i]);
    }
}

void uid_to_uname(char *name_buf, uid_t uid)
{
    struct passwd *pwd = getpwuid((uid_t) uid);

    if (pwd != NULL) {
        strncpy(name_buf, pwd->pw_name, 15);
    } else {
        sprintf(name_buf, "%d", uid);
    }
}
