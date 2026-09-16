#ifndef DISK_MONITOR_H
#define DISK_MONITOR_H

typedef struct {
    unsigned long long total_bytes;
    unsigned long long free_bytes;
    unsigned long long used_bytes;
    float usage_percent;
} disk_info_t;

int get_disk_info(const char *path, disk_info_t *info);

#endif
