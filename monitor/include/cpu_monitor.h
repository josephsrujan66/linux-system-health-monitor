#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

typedef struct {
    unsigned long long total_time;
    unsigned long long idle_time;
    float usage_percent;
} cpu_info_t;

int get_cpu_usage(cpu_info_t *info);

#endif
