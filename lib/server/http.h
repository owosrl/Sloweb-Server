#ifndef __HTTP_H__
#define __HTTP_H__

#include "tcp.h"
#include "config/config.h"

// HTTP status codes
#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_SERVER_ERROR 500


// HTTP packet format
typedef struct {
    char *method;
    char *path;
    char *version;
    char *body;
} http_request;

typedef struct {
    char version[10];
    int status;
    char status_msg[32];
    char server[32];
    char contentType[32];
    char contentLength[32];
    char *body;
} http_response;

// HTTP response methods

// Create a new HTTP response
http_response *http_response_new(tcp_socket *sock, server_config *config, http_request *request);

// Handle a HTTP request
int http_handle_request(tcp_socket *sock, server_config *config);

#endif