#ifndef MONITOR_ENGINE_H
#define MONITOR_ENGINE_H

#include <pthread.h>

#include "system_info.h"
#include "memory_monitor.h"
#include "cpu_monitor.h"
#include "disk_monitor.h"
#include "network_monitor.h"
#include <time.h>
typedef enum
{
    METRIC_STATE_UNKNOWN = 0,
    METRIC_STATE_VALID,
    METRIC_STATE_ERROR
} metric_state_t;

typedef struct
{
    metric_state_t state;
    time_t last_updated;

} metric_status_t;

typedef struct
{
    system_info_t system;

    memory_info_t memory;
    metric_status_t memory_status;

    cpu_info_t cpu;
    metric_status_t cpu_status;

    disk_info_t disk;
    metric_status_t disk_status;

    network_info_t network;
    metric_status_t network_status;

    pthread_mutex_t mutex;

} monitor_data_t;

int monitor_engine_init(monitor_data_t *data);
void monitor_engine_destroy(monitor_data_t *data);

int monitor_engine_start(monitor_data_t *data);

#endif
