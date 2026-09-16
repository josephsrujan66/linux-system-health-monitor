#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "monitor_engine.h"


static void *memory_thread(void *arg)
{
    monitor_data_t *data = (monitor_data_t *)arg;

    while (1)
    {
        memory_info_t memory;

	if (get_memory_info(&memory) == 0)
	{
	    pthread_mutex_lock(&data->mutex);

	    data->memory = memory;
	    data->memory_status.state = METRIC_STATE_VALID;
	    data->memory_status.last_updated = time(NULL);

	    pthread_mutex_unlock(&data->mutex);
	}
	else
	{
	    pthread_mutex_lock(&data->mutex);

	    data->memory_status.state = METRIC_STATE_ERROR;

	    pthread_mutex_unlock(&data->mutex);
	}
        sleep(5);
    }

    return NULL;
}


static void *cpu_thread(void *arg)
{
    monitor_data_t *data = (monitor_data_t *)arg;

    while (1)
    {
        cpu_info_t cpu;

	if (get_cpu_usage(&cpu) == 0)
	{
	    pthread_mutex_lock(&data->mutex);

	    data->cpu = cpu;
	    data->cpu_status.state = METRIC_STATE_VALID;
	    data->cpu_status.last_updated = time(NULL);

	    pthread_mutex_unlock(&data->mutex);
	}
	else
	{
	    pthread_mutex_lock(&data->mutex);

	    data->cpu_status.state = METRIC_STATE_ERROR;

	    pthread_mutex_unlock(&data->mutex);
	}

        sleep(5);
    }

    return NULL;
}


static void *network_thread(void *arg)
{
    monitor_data_t *data = (monitor_data_t *)arg;

    while (1)
    {
        network_info_t network;

	if (get_network_info(&network) == 0)
	{
	    pthread_mutex_lock(&data->mutex);

	    data->network = network;
	    data->network_status.state = METRIC_STATE_VALID;
	    data->network_status.last_updated = time(NULL);

	    pthread_mutex_unlock(&data->mutex);
	}
	else
	{
	    pthread_mutex_lock(&data->mutex);

	    data->network_status.state = METRIC_STATE_ERROR;

	    pthread_mutex_unlock(&data->mutex);
	}

        sleep(5);
    }

    return NULL;
}

static void *disk_thread(void *arg)
{
    monitor_data_t *data = (monitor_data_t *)arg;

    while (1)
    {
        disk_info_t disk;

        if (get_disk_info("/", &disk) == 0)
        {
            pthread_mutex_lock(&data->mutex);

            data->disk = disk;
            data->disk_status.state = METRIC_STATE_VALID;
            data->disk_status.last_updated = time(NULL);

            pthread_mutex_unlock(&data->mutex);
        }
        else
        {
            pthread_mutex_lock(&data->mutex);

            data->disk_status.state = METRIC_STATE_ERROR;

            pthread_mutex_unlock(&data->mutex);
        }

        sleep(5);
    }

    return NULL;
}


int monitor_engine_init(monitor_data_t *data)
{
    if (data == NULL)
        return -1;

    /*
     * Initialize system information
     */
    if (get_system_info(&data->system) != 0)
    {
        fprintf(
            stderr,
            "Failed to get system information\n"
        );

        return -1;
    }

    /*
     * Initialize metric states
     */
    data->memory_status.state = METRIC_STATE_UNKNOWN;
    data->memory_status.last_updated = 0;

    data->cpu_status.state = METRIC_STATE_UNKNOWN;
    data->cpu_status.last_updated = 0;

    data->disk_status.state = METRIC_STATE_UNKNOWN;
    data->disk_status.last_updated = 0;

    data->network_status.state = METRIC_STATE_UNKNOWN;
    data->network_status.last_updated = 0;

    /*
     * Initialize mutex
     */
    if (pthread_mutex_init(&data->mutex, NULL) != 0)
    {
        return -1;
    }

    return 0;
}


int monitor_engine_start(monitor_data_t *data)
{
    pthread_t memory_tid;
    pthread_t cpu_tid;
    pthread_t network_tid;
    pthread_t disk_tid;

    if (pthread_create(&memory_tid, NULL, memory_thread, data) != 0)
        return -1;

    if (pthread_create(&cpu_tid, NULL, cpu_thread, data) != 0)
        return -1;

    if (pthread_create(&network_tid, NULL, network_thread, data) != 0)
        return -1;
    if (pthread_create(&disk_tid, NULL, disk_thread, data) != 0)
    	return -1;

    pthread_detach(memory_tid);
    pthread_detach(cpu_tid);
    pthread_detach(network_tid);
    pthread_detach(disk_tid);

    return 0;
}


void monitor_engine_destroy(monitor_data_t *data)
{
    if (data == NULL)
        return;

    pthread_mutex_destroy(&data->mutex);
}
