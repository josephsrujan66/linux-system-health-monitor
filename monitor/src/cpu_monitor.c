#include <stdio.h>
#include <unistd.h>

#include "../include/cpu_monitor.h"


static int read_cpu_times(unsigned long long *total,
                          unsigned long long *idle)
{
    FILE *fp;
    char line[256];

    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle_time;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;

    fp = fopen("/proc/stat", "r");

    if (fp == NULL)
    {
        perror("fopen /proc/stat");
        return -1;
    }

    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    sscanf(line,
           "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user,
           &nice,
           &system,
           &idle_time,
           &iowait,
           &irq,
           &softirq,
           &steal);

    *total = user + nice + system + idle_time +
            iowait + irq + softirq + steal;

    *idle = idle_time + iowait;

    return 0;
}


int get_cpu_usage(cpu_info_t *info)
{
    unsigned long long total1;
    unsigned long long idle1;

    unsigned long long total2;
    unsigned long long idle2;

    unsigned long long total_diff;
    unsigned long long idle_diff;

    if (info == NULL)
        return -1;

    if (read_cpu_times(&total1, &idle1) != 0)
        return -1;

    sleep(1);

    if (read_cpu_times(&total2, &idle2) != 0)
        return -1;

    total_diff = total2 - total1;
    idle_diff = idle2 - idle1;

    if (total_diff == 0)
    {
        info->usage_percent = 0.0f;
    }
    else
    {
        info->usage_percent =
            ((float)(total_diff - idle_diff) /
             total_diff) * 100.0f;
    }

    info->total_time = total2;
    info->idle_time = idle2;

    return 0;
}
