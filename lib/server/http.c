#include "http.h"
#include "tcp.h"
#include "config/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

void http_response_free(http_response *res);
void http_request_free(http_request *req);

http_response *http_response_new(tcp_socket *sock, server_config *conf, http_request *request) {
    http_response *res = malloc(sizeof(http_response));
    strcpy (res->version, "HTTP/1.1");
    res->status = 200;
    strcpy(res->status_msg, "OK\r\n");
    strcpy(res->server, "Server: Schifo Server/1.0\r\n");

    // check mime type and set content type
    char *ext = strrchr(request->path, '.');
    if (ext == NULL) {
        strcpy(res->contentType, "Content-Type: text/html\r\n");
    } else if (strcmp(ext, ".html") == 0) {
        strcpy(res->contentType, "Content-Type: text/html\r\n");
    } else if (strcmp(ext, ".css") == 0) {
        strcpy(res->contentType, "Content-Type: text/css\r\n");
    } else if (strcmp(ext, ".js") == 0) {
        strcpy(res->contentType, "Content-Type: text/javascript\r\n");
    } else if (strcmp(ext, ".png") == 0) {
        strcpy(res->contentType, "Content-Type: image/png\r\n");
    } else if (strcmp(ext, ".jpg") == 0) {
        strcpy(res->contentType, "Content-Type: image/jpeg\r\n");
    } else if (strcmp(ext, ".gif") == 0) {
        strcpy(res->contentType, "Content-Type: image/gif\r\n");
    } else {
        strcpy(res->contentType, "Content-Type: text/plain\r\n");
    }


    char *content = malloc(sizeof(char) * MAX_CONTENT);

    if (strcmp(request->path, "/") == 0) {
        char *path = strcat(conf->root, conf->index);
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
            fprintf(stderr, "Error opening %s\n", conf->index);
            res->status = 404;
            strcpy(res->status_msg, "Not Found");
            strcpy(res->contentType, "");
            strcpy(res->contentLength, "");
            return res;
        }
        fread(content, sizeof(char), MAX_CONTENT, fp);
        printf("Content from /\n");
        fclose(fp);
    } else {
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0 || strcmp(ext, ".gif") == 0) {
            char *path = strcat(conf->root, request->path);
            int fp = open(path, O_RDONLY);
            if (fp == -1) {
                fprintf(stderr, "Error opening %s\n", request->path);
                res->status = 404;
                strcpy(res->status_msg, "Not Found\r\n");
                strcpy(res->contentType, "");
                strcpy(res->contentLength, "");
                return res;
            }
            // read image into content as binary
            read(fp, content, MAX_CONTENT);
            printf("Content from %s\n", path);
            close(fp);
        } else {
            char *path = strcat(conf->root, request->path);
            FILE *fp = fopen(path, "r");
            if (fp == NULL) {
                fprintf(stderr, "Error opening %s\n", request->path);
                res->status = 404;
                strcpy(res->status_msg, "Not Found");
                strcpy(res->contentType, "");
                strcpy(res->contentLength, "");
                return res;
            }
            fread(content, sizeof(char), 2048, fp);
            printf("Content from %s\n", path);
            fclose(fp);
        }
    }
    strcat(content, "\r\n");

    sprintf(res->contentLength, "Content-Length: %lu\r\n", strlen(content));
    res->body = content;

    return res;
}

int http_handle_request(tcp_socket *sock, server_config *conf) {
    // create new tcp packet
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

        // create new http request based on tcp packet
        http_request *request = malloc(sizeof(http_request));

        char raw_request[MAX_BUFF];
        read(fd_conn, raw_request, sizeof(raw_request));

        // parse raw_request in request "GET / HTTP/1.1"
        char *token = strtok(raw_request, " ");
        request->method = token;
        token = strtok(NULL, " ");
        request->path = token;
        token = strtok(NULL, "\r\n");
        request->version = token;


        // print request
        printf("%s %s %s\r\n", request->method, request->path, request->version);

        // create new http response based on request
        http_response *response = http_response_new(sock, conf, request);
        if (response == NULL) {
            printf("Response is null\n");
            return TCP_ERR;
        }

        char msg[MAX_BUFF];
        bzero(msg, sizeof(msg));

        time_t t_date = time(NULL);
        char date[38] = "Date: ";

        strcat(date, ctime(&t_date));

        // asseble version and status
        char status[16];
        sprintf(status, "%d", response->status);
        strcat(msg, response->version);
        strcat(msg, " ");
        strcat(msg, status);
        strcat(msg, " ");
        strcat(msg, response->status_msg);
        strcat(msg, date);
        strcat(msg, response->server);
        strcat(msg, response->contentType);
        strcat(msg, response->contentLength);
        strcat(msg, "Connection: close\r\n");
        strcat(msg, "\r\n");
        strcat(msg, response->body);
        write(fd_conn, msg, sizeof(msg));

        // free memory
        http_request_free(request);
        http_response_free(response);
    }

    return TCP_OK;
}

void http_response_free(http_response *res) {
    free(res->body);
    free(res);
}

void http_request_free(http_request *req) {
    free(req);
}