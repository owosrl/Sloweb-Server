#ifndef __TCP_H__
#define __TCP_H__

// ERROR CODES
#define TCP_OK              0
#define TCP_ERR             -1
#define TCP_ERR_SOCKET      -2
#define TCP_ERR_BIND        -3
#define TCP_ERR_LISTEN      -4
#define TCP_ERR_ACCEPT      -5
#define TCP_ERR_FILE        -6

#define MAX_BUFF            3096
#define MAX_CONTENT         2048

#include "config/config.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

typedef struct _tcp_socket {
    in_addr_t addr;
    in_port_t port;
    uint8_t  proto;

    int                 fd_sock;
    struct sockaddr_in  *addr_in;
} tcp_socket;

extern tcp_socket *new_socket(uint32_t addr, uint16_t port, uint8_t proto);
extern void close_socket(tcp_socket *sock);
extern void listen_socket(tcp_socket *sock, int backlog);
extern int test_connection(tcp_socket *sock);
extern int test_connection_index(tcp_socket *sock, server_config *conf);

#endif