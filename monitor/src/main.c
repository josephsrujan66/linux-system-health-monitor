#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "monitor_engine.h"

static void print_timestamp(time_t timestamp)
{
    char buffer[64];

    if (timestamp == 0)
    {
        printf("Never");
        return;
    }

    struct tm *tm_info = localtime(&timestamp);

    if (tm_info == NULL)
    {
        printf("Invalid");
        return;
    }

    strftime(buffer, sizeof(buffer),
             "%Y-%m-%d %H:%M:%S",
             tm_info);

    printf("%s", buffer);
}


static const char *state_to_string(metric_state_t state)
{
    switch (state)
    {
        case METRIC_STATE_VALID:
            return "VALID";

        case METRIC_STATE_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}



int main(void)
{
    monitor_data_t data;

    if (monitor_engine_init(&data) != 0)
    {
        fprintf(stderr, "Failed to initialize monitor engine\n");
        return 1;
    }

    if (monitor_engine_start(&data) != 0)
    {
        fprintf(stderr, "Failed to start monitor engine\n");
        monitor_engine_destroy(&data);
        return 1;
    }

    printf("Linux System Monitor started...\n");
    printf("Monitoring threads are running.\n");

    while (1)
    {

	pthread_mutex_lock(&data.mutex);

	printf("\n===== MONITOR DATA =====\n");

	printf("CPU Usage : %.2f%%\n", data.cpu.usage_percent);
	printf("CPU State : %s\n",
	       state_to_string(data.cpu_status.state));
	printf("CPU Updated : ");
	print_timestamp(data.cpu_status.last_updated);
	printf("\n");

	printf("\nRAM Usage : %.2f%%\n", data.memory.usage_percent);
	printf("RAM State : %s\n",
	       state_to_string(data.memory_status.state));
	printf("RAM Updated : ");
	print_timestamp(data.memory_status.last_updated);
	printf("\n");

	printf("\nDisk Usage : %.2f%%\n", data.disk.usage_percent);
	printf("Disk State : %s\n",
	       state_to_string(data.disk_status.state));
	printf("Disk Updated : ");
	print_timestamp(data.disk_status.last_updated);
	printf("\n");

	printf("\nNetwork State : %s\n",
	       state_to_string(data.network_status.state));
	printf("Network Updated : ");
	print_timestamp(data.network_status.last_updated);
	printf("\n");

	pthread_mutex_unlock(&data.mutex);

        sleep(5);
    }

    monitor_engine_destroy(&data);

    return 0;
}
