#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

typedef struct {
    unsigned long total_kb;
    unsigned long free_kb;
    unsigned long available_kb;
    unsigned long used_kb;
    float usage_percent;
} memory_info_t;

int get_memory_info(memory_info_t *info);

#endif
