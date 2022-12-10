#include "tcp.h"
#include "http.h"
#include "config/config.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>


/*
    TCP Socket Methods

    tcp_socket *new_socket  -> create a new socket
    void       close_socket -> close a given socket
*/

tcp_socket *new_socket(in_addr_t addr, in_port_t port, uint8_t proto) {
    tcp_socket *sock = malloc(sizeof(tcp_socket));
    sock->addr = addr;
    sock->port = port;
    sock->proto = proto;
    sock->fd_sock = socket(sock->proto, SOCK_STREAM, 0);

    sock->addr_in = malloc(sizeof(struct sockaddr_in));
    memset(sock->addr_in, 0, sizeof(struct sockaddr_in));

    if (sock->fd_sock < 0) {
        fprintf(stderr, "Error opening socket: %d", TCP_ERR_SOCKET);
        return NULL;
    }

    sock->addr_in->sin_family = sock->proto;
    sock->addr_in->sin_port = htons(sock->port);
    sock->addr_in->sin_addr.s_addr = sock->addr;

    if (bind(sock->fd_sock, (struct sockaddr *) sock->addr_in, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Error binding socket: %d", TCP_ERR_BIND);
        return NULL;
    }

    if (sock->port == 0) {
        socklen_t len = sizeof(sock->addr_in);
        if (getsockname(sock->fd_sock, (struct sockaddr *) sock->addr_in, &len) < 0) {
            fprintf(stderr, "Error getting socket name: %d", TCP_ERR_SOCKET);
            return NULL;
        }
        sock->port = ntohs(sock->addr_in->sin_port);
    }

    return sock;
}

void close_socket(tcp_socket *sock) {
    close(sock->fd_sock);
    free(sock->addr_in);
    free(sock);
}

int getproto(char *proto) {
    if (strcmp(proto, "IPv4") == 0) {
        return AF_INET;
    } else if (strcmp(proto, "IPv6") == 0) {
        return AF_INET6;
    }
    return AF_INET;
}

/*
    Connection-related Method

    int test_connection -> verify socket connection and respond 200 OK to client
*/

int test_connection(tcp_socket *sock) {
    if (listen(sock->fd_sock, 5) < 0) {
        perror("Error listening");
        return TCP_ERR_LISTEN;
    } else {
        printf("Listening on %s:%d\n", inet_ntoa(sock->addr_in->sin_addr), ntohs(sock->addr_in->sin_port));
    }

    uint32_t len;
    struct sockaddr_in *addr_out = malloc(sizeof(struct sockaddr_in));
    memset(addr_out, 0, sizeof(struct sockaddr_in));

    len = sizeof(addr_out);

    int fd_conn = accept(sock->fd_sock, (struct sockaddr *) addr_out, &len);
    if (fd_conn < 0) {
        perror("Error accepting connection");
        return TCP_ERR_ACCEPT;
    } else {
        printf("Connection accepted from %s:%d\n", inet_ntoa(addr_out->sin_addr), ntohs(addr_out->sin_port));
        // send 200 OK
        char msg[MAX_BUFF];
        memset(msg, 0, sizeof(msg));

        char content[MAX_CONTENT];
        memset(content, 0, sizeof(content));
        strcpy(content, "Connection OK\r\n");

        time_t t_date = time(NULL);
        char date[38] = "Date: ";

        strcat(date, ctime(&t_date));

        int len = strlen(content);
        char lenitoa[8];
        sprintf(lenitoa, "%d", len);
        char len_str[28] = "Content-Length: ";
        strcat(len_str, lenitoa);
        strcat(len_str, "\r\n");

        strcpy(msg, "HTTP/1.1 200 OK\r\n");
        strcat(msg, date);
        strcat(msg, "Server: Schifo Server/1.0\r\n");
        strcat(msg, "Content-type: text/html\r\n");
        strcat(msg, len_str);
        strcat(msg, "Connection: close\r\n");
        strcat(msg, "\r\n");
        strcat(msg, content);
        write(fd_conn, msg, sizeof(msg));
    }

    return 0;
}

int test_connection_index(tcp_socket *sock, server_config *conf) {
    if (listen(sock->fd_sock, 5) < 0) {
        perror("Error listening");
        return TCP_ERR_LISTEN;
    } else {
        printf("Listening on %s:%d\n", inet_ntoa(sock->addr_in->sin_addr), ntohs(sock->addr_in->sin_port));
    }

    uint32_t len;
    struct sockaddr_in *addr_out = malloc(sizeof(struct sockaddr_in));
    memset(addr_out, 0, sizeof(struct sockaddr_in));

    len = sizeof(addr_out);

    int fd_conn = accept(sock->fd_sock, (struct sockaddr *) addr_out, &len);
    if (fd_conn < 0) {
        perror("Error accepting connection");
        return TCP_ERR_ACCEPT;
    } else {
        printf("Connection accepted from %s:%d\n", inet_ntoa(addr_out->sin_addr), ntohs(addr_out->sin_port));
        // send 200 OK
        char msg[MAX_BUFF];
        memset(msg, 0, sizeof(msg));

        char content[MAX_CONTENT];
        memset(content, 0, sizeof(content));
        char *path = strcat(conf->root, conf->index);


        // open index.html
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
            fprintf(stderr, "Error opening %s\n", path);
            return TCP_ERR_FILE;
        }

        // read index.html in content
        fread(content, sizeof(char), MAX_CONTENT, fp);
        strcat(content, "\r\n");
        fclose(fp);


        time_t t_date = time(NULL);
        char date[38] = "Date: ";

        strcat(date, ctime(&t_date));

        int len = strlen(content);
        char lenitoa[8];
        sprintf(lenitoa, "%d", len);
        char len_str[28] = "Content-Length: ";
        strcat(len_str, lenitoa);
        strcat(len_str, "\r\n");

        strcpy(msg, "HTTP/1.1 200 OK\r\n");
        strcat(msg, date);
        strcat(msg, "Server: Schifo Server/1.0\r\n");
        strcat(msg, "Content-type: text/html\r\n");
        strcat(msg, len_str);
        strcat(msg, "Connection: close\r\n");
        strcat(msg, "\r\n");
        strcat(msg, content);
        write(fd_conn, msg, sizeof(msg));
    }

    return 0;
}


// listen connections and handle them in a new thread for each connection
// responde with index.html in root directory
int listen_connections(tcp_socket *sock, server_config *conf) {
    pid_t pid;
    
    while (1) {
        pid = fork();
        if (pid == 0) {
            http_handle_request(sock, conf);
            return 0;
        } else {
            wait(NULL);
        }
    }
}