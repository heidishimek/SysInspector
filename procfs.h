/**
 * @file
 *
 * Retrieves raw information from procfs, parses it, and provides easy-to-use
 * functions for displaying the data.
 */

#ifndef _PROCFS_H_
#define _PROCFS_H_

#include <sys/types.h>

struct load_avg {
    double one;
    double five;
    double fifteen;
};

struct cpu_stats {
    unsigned long idle;
    unsigned long usage;
    float usage_perc;
};

struct mem_stats {
    double used;
    double total;
};

/**
 * Overall task statistics 
 */
struct task_stats {
    size_t total;
    size_t running;
    size_t waiting;
    size_t sleeping;
    size_t stopped;
    size_t zombie;

    struct task_info *active_tasks;
};

/**
 * Individual tasks' info
 */
struct task_info {
    pid_t pid;
    uid_t uid;
    char name[26];
    char state[13];
};

int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz);
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz);
int pfs_cpu_units(char *proc_dir);
double pfs_uptime(char *proc_dir);
int pfs_format_uptime(double time, char *uptime_buf);

struct load_avg pfs_load_avg(char *proc_dir);
/* Note: 'prev' is an input that represents the *last* CPU usage state. 'curr'
 * is an *output* that represents the current CPU usage state, and will be
 * passed back in to pfs_load_avg on its next call. */
void pfs_cpu_usage(
        char *procfs_dir, struct cpu_stats *prev, struct cpu_stats *curr);
struct mem_stats pfs_mem_usage(char *procfs_dir);

/* Note: these functions create and destroy task_stats structs. Depending on
 * your implementation, this might just be a malloc/free. */
struct task_stats *pfs_create_tstats();
void pfs_destroy_tstats(struct task_stats *tstats);

/* Note: much like pfs_cpu_usage, 'prev_*' are inputs that represents the *last*
 * CPU usage and task state. 'curr_*' are *outputs* that represent the current
 * CPU usage and task state, and will be passed back in to pfs_tasks next call.
 */
void pfs_tasks(char *proc_dir, struct task_stats *stats);

#endif
