#ifndef MAASKK_HTTPD_H
#define MAASKK_HTTPD_H

#include <stdbool.h>
#include <stdint.h>

struct server_config {
    const char *address;
    uint16_t port;
    int backlog;
    bool once;
};

struct server {
    int listener_fd;
    struct server_config config;
};

int server_init(struct server *server, const struct server_config *config);
int server_run(struct server *server);
void server_close(struct server *server);

#endif

