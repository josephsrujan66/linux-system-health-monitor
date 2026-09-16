#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>

#include "../include/network_monitor.h"


/*
 * Read RX and TX bytes from /proc/net/dev
 */
static void get_interface_traffic(const char *interface_name,
                                   unsigned long long *rx_bytes,
                                   unsigned long long *tx_bytes)
{
    FILE *fp;
    char line[512];

    char name[32];

    unsigned long long rx;
    unsigned long long tx;

    *rx_bytes = 0;
    *tx_bytes = 0;

    fp = fopen("/proc/net/dev", "r");

    if (fp == NULL)
    {
        perror("fopen /proc/net/dev");
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        /*
         * Format:
         *
         * interface:
         * RX bytes packets errs drop fifo frame compressed multicast
         * TX bytes packets errs drop fifo colls carrier compressed
         */

        if (sscanf(line,
                   " %31[^:]: %llu %*u %*u %*u %*u %*u %*u %*u "
                   "%llu",
                   name,
                   &rx,
                   &tx) == 3)
        {
            if (strcmp(name, interface_name) == 0)
            {
                *rx_bytes = rx;
                *tx_bytes = tx;
                break;
            }
        }
    }

    fclose(fp);
}


int get_network_info(network_info_t *info)
{
    struct ifaddrs *interfaces;
    struct ifaddrs *ifa;

    if (info == NULL)
        return -1;

    info->interface_count = 0;

    if (getifaddrs(&interfaces) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = interfaces;
         ifa != NULL && info->interface_count < MAX_INTERFACES;
         ifa = ifa->ifa_next)
    {
        /*
         * Skip invalid entries
         */
        if (ifa->ifa_addr == NULL)
            continue;

        /*
         * We only process IPv4 addresses
         */
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        network_interface_t *interface =
            &info->interfaces[info->interface_count];

        /*
         * Copy interface name
         */
        snprintf(interface->name,
                 sizeof(interface->name),
                 "%s",
                 ifa->ifa_name);

        /*
         * Convert binary IP address to string
         */
        struct sockaddr_in *addr =
            (struct sockaddr_in *)ifa->ifa_addr;

        inet_ntop(AF_INET,
                  &addr->sin_addr,
                  interface->ip_address,
                  sizeof(interface->ip_address));

        /*
         * Check interface status
         */
        interface->is_up =
            (ifa->ifa_flags & IFF_UP) ? 1 : 0;

        /*
         * Read traffic statistics
         */
        get_interface_traffic(interface->name,
                               &interface->rx_bytes,
                               &interface->tx_bytes);

        info->interface_count++;
    }

    freeifaddrs(interfaces);

    return 0;
}
