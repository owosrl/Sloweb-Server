#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <iniparser.h>

typedef struct _server_config {
    char *addr;
    uint16_t port;
    char *root;
    char *index;
} server_config;

extern dictionary *open_config(const char *filename);
extern void close_config(dictionary *config);
extern server_config *parse_config(dictionary *config);
extern void print_config(server_config *config);

#endif