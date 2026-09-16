#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

typedef struct {
    char hostname[256];
    char os_name[256];
    char kernel_version[256];
    char architecture[256];
} system_info_t;

int get_system_info(system_info_t *info);

#endif
