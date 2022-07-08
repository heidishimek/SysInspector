#include "logger.h"
#include "procfs.h"
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "util.h"

int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{
    int fds[2];
    if (proc_open(proc_dir, "sys/kernel/hostname", fds) == -1) {
        perror("proc_open");
        return -1;
    }

    ssize_t bytes = lineread(fds[1], hostname_buf, buf_sz);
    proc_close(fds);
    if (bytes == -1) {
        return -1;
    }

    hostname_buf[bytes - 1] = '\0';

    LOG("hostname we read: '%s'\n", hostname_buf);
    return 0;
}

int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
    int fds[2];
    if (proc_open(proc_dir, "sys/kernel/osrelease", fds) == -1) {
        perror("proc_open");
        return -1;
    }

    char *temp = dynamic_lineread(fds[1]);
    proc_close(fds);
    if (temp == NULL) {
        return -1;
    }

    size_t dash_location = strcspn(temp, "-");
    temp[dash_location] = '\0';

    LOG("version we read: '%s'\n", temp);

    strncpy(version_buf, temp, buf_sz);
    free(temp);
    return 0;
}

int pfs_cpu_units(char *proc_dir)
{
    int fds[2];
    if (proc_open(proc_dir, "cpuinfo", fds) == -1) {
        perror("proc_open");
        return -1;
    }
    char temp[128];
    int count = 0;
    ssize_t size = 0;

    while ((size = lineread(fds[1], temp, 128)) > 0) {
        if (strncmp(temp, "processor", 2) == 0) {
            count++;
        }
    }

    proc_close(fds);
    return count;
}

double pfs_uptime(char *proc_dir)
{
    int fds[2];
    if (proc_open(proc_dir, "uptime", fds) == -1) {
        perror("proc_open");
        return -1;
    }

    char temp[128];
    char *endptr = {0};
    double uptime;
    if (temp != endptr) {
        lineread(fds[1], temp, 128);

    }

    uptime = strtod(temp, &endptr);

    proc_close(fds);
    return uptime;
}

int pfs_format_uptime(double time, char *uptime_buf)
{
    int days = (int) time / 86400;
    int hours = (int) (time / 3600) % 24;
    int minutes = (int) (time / 60) % 60;
    int seconds = (int) time % 60;

    if (days == 0) {
        sprintf(uptime_buf, "%02d:%02d:%02d", hours, minutes, seconds);
    } else if (days == 1) {
        sprintf(uptime_buf, "%d day, %02d:%02d:%02d", days, hours, minutes, seconds);
    } else {
        sprintf(uptime_buf, "%d days, %02d:%02d:%02d", days, hours, minutes, seconds);
    }

    return 0;
}

struct load_avg pfs_load_avg(char *proc_dir)
{
    int fds[2];
    struct load_avg lavg = { 0 };
    if (proc_open(proc_dir, "loadavg", fds) == -1) {
        perror("proc_open");
        return lavg; // return an empty struct if errors occur
    }

    char temp[128];
    lineread(fds[1], temp, 128);
    char *tok = temp;
    char *token;
    
    token = next_token(&tok, " ");
    lavg.one = strtod(token, &token);
    token = next_token(&tok, " ");
    lavg.five = strtod(token, &token);
    token = next_token(&tok, " ");
    lavg.fifteen = strtod(token, &token);

    proc_close(fds);
    return lavg;
}

void pfs_cpu_usage(char *proc_dir,
        struct cpu_stats *prev, struct cpu_stats *curr)
{
    // TODO: calculate CPU usage. This involves reading the various counters for
    // the CPU (user, system, idle, etc. usage), storing the data in a cpu_stats
    // struct, and then comparing current usage with the previous.
    //
    // The CPU is only either *in use* or *idle*, so the CPU % is actually the
    // amount of time the CPU was not idle over a given period. Therefore, we
    // can calculate the percentage with:
    //
    // (usage2 - usage1) / (total2 - total1)

    int fds[2];
    if (proc_open(proc_dir, "loadavg", fds) == -1) {
        perror("proc_open");
    }

    // double usage1, usage2, total1, total2;



    proc_close(fds);
}

struct mem_stats pfs_mem_usage(char *proc_dir)
{
    // TODO: determine the total memory available on this system and how much is
    // being used. You will want to look at the **total** and **available**
    // memory in procfs.

    struct mem_stats mstats = { 0 };
    return mstats; // return an empty struct if errors occur
}

struct task_stats *pfs_create_tstats()
{
    struct task_stats *tstats = calloc(1, sizeof(struct task_stats));
    tstats->active_tasks = calloc(1, sizeof(struct task_stats));
    return tstats;
}

void pfs_destroy_tstats(struct task_stats *tstats)
{
    free(tstats->active_tasks);
    free(tstats);
}

void pfs_tasks(char *proc_dir, struct task_stats *tstats)
{
    // TODO: 1. Loop through all the directories in /proc
    //       2. Find the ones that are numbers only (those are the processes)
    //       3. Figure out their name, PID, state, and owner, and whether they
    //          are an 'active' task (not idle or sleeping)
    //       4. Store the info in the task_stats struct
}
