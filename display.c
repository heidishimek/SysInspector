#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <pwd.h>

#include "logger.h"
#include "display.h"
#include "procfs.h"
#include "util.h"

#define DISP_PRINTMV(y, x, fmt, ...) \
    do { \
        if (use_curses) { \
            mvprintw(y, x, fmt, __VA_ARGS__); \
            clrtoeol(); \
        } else { \
            printf(fmt, __VA_ARGS__); \
        } \
    } while (0) 

#define DISP_PRINT(fmt, ...) \
    do { \
        if (use_curses) { \
            printw(fmt, __VA_ARGS__); \
            clrtoeol(); \
        } else { \
            printf(fmt, __VA_ARGS__); \
        } \
    } while (0) 

#define DISP_BLUE  1
#define DISP_GREEN 2
#define DISP_RED   3

void display_separator(void);

static WINDOW *scr;
static int win_x;
static int win_y;
static bool use_curses = true;

void display_init(bool enable_curses)
{
    LOGP("Initializing the display...\n");

    if (enable_curses == false) {
        LOGP("Curses-based display disabled\n");
        use_curses = false;
        win_x = 80;
        win_y = 24;
        return;
    }

    LOGP("Attempting to redirect stderr to inspector.log\n");
    int fd = open("inspector.log", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
    }

    int ret = dup2(fd, fileno(stderr));
    if (ret == -1) {
        perror("dup2");
    }

    scr = initscr();
    use_default_colors();
    start_color();

    init_pair(DISP_BLUE,  COLOR_BLUE, -1);
    init_pair(DISP_GREEN, COLOR_GREEN, -1);
    init_pair(DISP_RED,   COLOR_RED, -1);

    getmaxyx(scr, win_y, win_x);
    LOG("Created curses screen. Dimensions: %d x %d\n", win_x, win_y);

    /* Make functions such as getch() return immediately without waiting for
     * user input: */
    nodelay(scr, true);

    /* Do not print characters typed by the user: */
    noecho();

    /* Hide the cursor: */
    curs_set(0);
}

void display_separator(void)
{
    if (use_curses == false) {
        puts("\n");
    }
}

void display_refresh(char *procfs_loc)
{
    int retval = 0;

    DISP_PRINTMV(0, 0, "%s", "Hostname: ");
    char name[HOST_NAME_MAX + 1];
    retval = pfs_hostname(procfs_loc, name, HOST_NAME_MAX + 1);
    if (retval == -1) {
        strcpy(name, "(UNKNOWN)");
    }
    attron(COLOR_PAIR(DISP_BLUE));
    DISP_PRINT("%s", name);
    attroff(COLOR_PAIR(DISP_BLUE));

    DISP_PRINT("%s", " | Kernel Version: ");
    char vers[32];
    retval = pfs_kernel_version(procfs_loc, vers, 32);
    if (retval == -1) {
        strcpy(vers, "(UNKNOWN)");
    }
    attron(COLOR_PAIR(DISP_RED));
    DISP_PRINT("%s", vers);
    attroff(COLOR_PAIR(DISP_RED));

    DISP_PRINT("%s", " | CPUs: ");
    int proc_units = pfs_cpu_units(procfs_loc);
    attron(COLOR_PAIR(DISP_BLUE));
    DISP_PRINT("%d\n", proc_units);
    attroff(COLOR_PAIR(DISP_BLUE));

    double uptime = pfs_uptime(procfs_loc);
    char uptime_str[24];
    if (uptime == 0.0) {
        strcpy(uptime_str, "(UNKNOWN)");
    } else {
        pfs_format_uptime(uptime, uptime_str);
    }
    DISP_PRINTMV(1, 0, "Uptime: %s", uptime_str);

    display_separator();

    struct load_avg lavg = pfs_load_avg(procfs_loc);
    char *header = "Load Average (1/5/15 min): ";
    DISP_PRINTMV(3, 0, "%s", header);
    attron(COLOR_PAIR(DISP_BLUE));
    DISP_PRINT("%.2f ", lavg.one);

    attron(COLOR_PAIR(DISP_GREEN));
    DISP_PRINT("%.2f ", lavg.five);

    attron(COLOR_PAIR(DISP_RED));
    DISP_PRINT("%.2f \n", lavg.fifteen);

    attroff(COLOR_PAIR(DISP_RED));
 
    char cpu_bar[30] = { 0 };
    static struct cpu_stats prev_cpu;
    struct cpu_stats curr_cpu;
    pfs_cpu_usage(procfs_loc, &prev_cpu, &curr_cpu);
    display_percbar(cpu_bar, curr_cpu.usage_perc);
    DISP_PRINTMV(4, 0, "CPU Usage:    %s", cpu_bar);

    char mem_bar[30] = { 0 };
    struct mem_stats mstats = pfs_mem_usage(procfs_loc);
    display_percbar(mem_bar, mstats.used / mstats.total);

    char used_str[10] = { 0 };
    human_readable_size(used_str, 10, mstats.used, 1);

    char total_str[10] = { 0 };
    human_readable_size(total_str, 10, mstats.total, 1);

    DISP_PRINTMV(5, 0, "Memory Usage: %s (%s / %s)", mem_bar, used_str, total_str);

    display_separator();

    struct task_stats *tstats = pfs_create_tstats();
    if (tstats == NULL) {
        DISP_PRINTMV(7, 0, "%s", "Tasks: (UNKNOWN)");
        goto cleanup;
    }

    pfs_tasks(procfs_loc, tstats);

    DISP_PRINTMV(7, 0, "Tasks: %zu total\n", tstats->total);
    DISP_PRINTMV(8, 0, "%zu running, %zu waiting, %zu sleeping, %zu stopped, %zu zombie",
            tstats->running,
            tstats->waiting,
            tstats->sleeping,
            tstats->stopped,
            tstats->zombie);

    display_separator();

    DISP_PRINTMV(11, 0, "%9s | %25s | %12s | %15s", "PID", "Task Name", "State", "User");
    DISP_PRINTMV(12, 0, "%s",
            "----------+---------------------------+--------------+-----------------");

    for (int i = 0; i < tstats->total - tstats->sleeping; ++i) {
        char uname[16];
        uid_to_uname(uname, tstats->active_tasks[i].uid);
        DISP_PRINTMV(13 + i, 0, "%9d | %25s | %12s | %15s",
                tstats->active_tasks[i].pid,
                tstats->active_tasks[i].name,
                tstats->active_tasks[i].state,
                uname);
    }
    pfs_destroy_tstats(tstats);

cleanup:
    prev_cpu = curr_cpu;
    if (use_curses == true) {
        clrtobot();
        refresh();
    }
}

bool display_quit(void)
{
    int ch = getch();
    if (ch != ERR && ch != KEY_RESIZE) {
        return true;
    } else {
        return false;
    }
}

void display_stop(void)
{
    /* Restore the cursor: */
    curs_set(1);
    endwin();
}

