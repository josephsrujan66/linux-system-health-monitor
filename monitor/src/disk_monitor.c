#include <stdio.h>
#include <sys/statvfs.h>

#include "../include/disk_monitor.h"


int get_disk_info(const char *path, disk_info_t *info)
{
    struct statvfs disk;

    if (path == NULL || info == NULL)
        return -1;

    if (statvfs(path, &disk) != 0)
    {
        perror("statvfs");
        return -1;
    }

    /*
     * Calculate filesystem sizes in bytes.
     *
     * f_frsize is the fundamental filesystem block size.
     */

    info->total_bytes =
        (unsigned long long)disk.f_blocks * disk.f_frsize;

    info->free_bytes =
        (unsigned long long)disk.f_bfree * disk.f_frsize;

    info->used_bytes =
        info->total_bytes - info->free_bytes;

    if (info->total_bytes > 0)
    {
        info->usage_percent =
            ((float)info->used_bytes / info->total_bytes) * 100.0f;
    }
    else
    {
        info->usage_percent = 0.0f;
    }

    return 0;
}
