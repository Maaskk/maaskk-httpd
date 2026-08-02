#include "httpd.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char response[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: 19\r\n"
    "Connection: close\r\n"
    "\r\n"
    "maaskk-httpd alive\n";

static int send_all(int fd, const char *buffer, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t count = send(fd, buffer + sent, length - sent, 0);
        if (count > 0) {
            sent += (size_t)count;
            continue;
        }
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count == 0) {
            fprintf(stderr, "send: peer closed before response completed\n");
        } else {
            perror("send");
        }
        return -1;
    }

    return 0;
}

static int serve_client(int client_fd)
{
    char request[4096];
    ssize_t count;

    do {
        count = recv(client_fd, request, sizeof(request), 0);
    } while (count < 0 && errno == EINTR);

    if (count < 0) {
        perror("recv");
        return -1;
    }
    if (count == 0) {
        fprintf(stderr, "recv: peer closed without sending a request\n");
        return -1;
    }

    return send_all(client_fd, response, sizeof(response) - 1);
}

int server_init(struct server *server, const struct server_config *config)
{
    struct sockaddr_in address = {0};
    int reuse = 1;

    if (server == NULL || config == NULL || config->address == NULL) {
        fprintf(stderr, "server_init: invalid configuration\n");
        return -1;
    }

    server->listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listener_fd < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(server->listener_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
        perror("setsockopt(SO_REUSEADDR)");
        server_close(server);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(config->port);
    if (inet_pton(AF_INET, config->address, &address.sin_addr) != 1) {
        fprintf(stderr, "inet_pton: invalid IPv4 address: %s\n", config->address);
        server_close(server);
        return -1;
    }

    if (bind(server->listener_fd, (const struct sockaddr *)&address, sizeof(address)) != 0) {
        perror("bind");
        server_close(server);
        return -1;
    }

    if (listen(server->listener_fd, config->backlog) != 0) {
        perror("listen");
        server_close(server);
        return -1;
    }

    server->config = *config;
    return 0;
}

int server_run(struct server *server)
{
    unsigned long clients_served = 0;

    if (server == NULL || server->listener_fd < 0) {
        fprintf(stderr, "server_run: server is not initialized\n");
        return -1;
    }

    printf("maaskk-httpd listening on http://%s:%u\n",
           server->config.address, (unsigned int)server->config.port);
    fflush(stdout);

    for (;;) {
        int client_fd = accept(server->listener_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            return -1;
        }

        int client_result = serve_client(client_fd);
        if (close(client_fd) != 0) {
            perror("close(client)");
            client_result = -1;
        }
        if (client_result != 0) {
            return -1;
        }

        ++clients_served;
        if (server->config.once && clients_served == 1) {
            return 0;
        }
    }
}

void server_close(struct server *server)
{
    if (server != NULL && server->listener_fd >= 0) {
        if (close(server->listener_fd) != 0) {
            perror("close(listener)");
        }
        server->listener_fd = -1;
    }
}

