#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "../include/system_info.h"


int get_system_info(system_info_t *info)
{
    struct utsname system_data;

    if (info == NULL)
        return -1;

    /* Get hostname */
    if (gethostname(info->hostname, sizeof(info->hostname)) != 0)
    {
        perror("gethostname");
        return -1;
    }

    /* Get kernel and architecture information */
    if (uname(&system_data) != 0)
    {
        perror("uname");
        return -1;
    }

    snprintf(info->kernel_version,
             sizeof(info->kernel_version),
             "%s",
             system_data.release);

    snprintf(info->architecture,
             sizeof(info->architecture),
             "%s",
             system_data.machine);

    /* Read OS information */
    FILE *fp = fopen("/etc/os-release", "r");

    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0)
        {
            char *value = line + 12;

            value[strcspn(value, "\n")] = '\0';

            /* Remove quotation marks */
            if (value[0] == '"')
            {
                memmove(value, value + 1, strlen(value));

                size_t len = strlen(value);

                if (len > 0 && value[len - 1] == '"')
                    value[len - 1] = '\0';
            }

            snprintf(info->os_name,
                     sizeof(info->os_name),
                     "%s",
                     value);

            break;
        }
    }

    fclose(fp);

    return 0;
}
