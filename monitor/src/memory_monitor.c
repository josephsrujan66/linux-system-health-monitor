#include <stdio.h>
#include <string.h>

#include "../include/memory_monitor.h"


int get_memory_info(memory_info_t *info)
{
    FILE *fp;
    char line[256];

    if (info == NULL)
        return -1;

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
    {
        perror("fopen /proc/meminfo");
        return -1;
    }

    info->total_kb = 0;
    info->free_kb = 0;
    info->available_kb = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "MemTotal: %lu kB", &info->total_kb) == 1)
            continue;

        if (sscanf(line, "MemFree: %lu kB", &info->free_kb) == 1)
            continue;

        if (sscanf(line, "MemAvailable: %lu kB", &info->available_kb) == 1)
            continue;
    }

    fclose(fp);

    /*
     * Used RAM = Total RAM - Available RAM
     */
    info->used_kb = info->total_kb - info->available_kb;

    if (info->total_kb > 0)
    {
        info->usage_percent =
            ((float)info->used_kb / info->total_kb) * 100.0f;
    }
    else
    {
        info->usage_percent = 0.0f;
    }

    return 0;
}
