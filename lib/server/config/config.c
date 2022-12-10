#include <iniparser.h>
#include <string.h>

#include "config.h"

// open given config.ini file
dictionary *open_config(const char *filename) {
    dictionary *config = iniparser_load(filename);
    if (config == NULL) {
        fprintf(stderr, "Error opening config file: %s\n", filename);
        return NULL;
    }
    return config;
}

// parse config file and return a config struct
server_config *parse_config(dictionary *config) {
    if (config == NULL) {
        return NULL;
    }

    server_config *cfg = malloc(sizeof(server_config));
    cfg->addr = (char *)iniparser_getstring(config, "server:ip", NULL);
    cfg->port = iniparser_getint(config, "server:port", -1);
    cfg->root = (char *)iniparser_getstring(config, "server:root", NULL);
    cfg->index = (char *)iniparser_getstring(config, "server:index", NULL);

    return cfg;
}

// close config file
void close_config(dictionary *config) {
    iniparser_freedict(config);
}


// print config file
void print_config(server_config *config) {
    printf("Port: %d\nIP: %s\nRoot: %s\nIndex: %s\n", config->port, config->addr, config->root, config->index);
}