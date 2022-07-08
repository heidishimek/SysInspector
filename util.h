#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/types.h>

int proc_open(char *proc_dir, char *file_path, int fds[2]);
int proc_close(int fds[2]);
ssize_t lineread(int fd, char *buf, size_t sz);
char *dynamic_lineread(int fd);
ssize_t one_lineread(int fd, char *buf, size_t sz);
char *next_token(char **str_ptr, const char *delim);
void display_percbar(char *buf, double frac);
void human_readable_size(char *buf, size_t buf_sz,
        double size, unsigned int decimals);
void uid_to_uname(char *name_buf, uid_t uid);

#endif
