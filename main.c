#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "lib/server/tcp.h"
#include "lib/server/config/config.h"

void print_usage(char *name) {
    printf("Usage: %s [<ip>] [<port>]\n", name);
}

int main(int argc, char *argv[]) {
    tcp_socket *sock;
    server_config *conf;

    switch (argc) {
        case 1:
            // check if config file exists and use it
            conf = parse_config(open_config("config.ini"));
            if (conf == NULL) {
                sock = new_socket(INADDR_ANY, 0, AF_INET);
            } else {
                print_config(conf);
                sock = new_socket(inet_addr(conf->addr), conf->port, getproto(conf->proto));
            }
            break;
        case 2:
            sock = new_socket(inet_addr(argv[1]), 0, AF_INET);
            break;
        case 3:
            sock = new_socket(inet_addr(argv[1]), atoi(argv[2]), AF_INET);
            break;
        default:
            print_usage(argv[0]);
            return 0;
    }
    
    if (sock == NULL) {
        close_socket(sock);
        printf("Error creating socket\n");
        return -1;
    }

    if (conf == NULL) {
        if (test_connection(sock) < 0) {
            printf("Error testing connection\n");
            close_socket(sock);
            return -1;
        }
    } else {
        if (listen_connections(sock, conf) < 0) {
            close_socket(sock);
            printf("Error testing connection\n");
            return -1;
        }
    }

    close_socket(sock);

    return 0;
}