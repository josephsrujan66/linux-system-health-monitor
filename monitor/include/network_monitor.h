#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include <netinet/in.h>

#define MAX_INTERFACES 32

typedef struct {
    char name[32];
    char ip_address[INET6_ADDRSTRLEN];

    int is_up;

    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
} network_interface_t;


typedef struct {
    network_interface_t interfaces[MAX_INTERFACES];
    int interface_count;
} network_info_t;


int get_network_info(network_info_t *info);

#endif
