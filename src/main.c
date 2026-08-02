#include "httpd.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE *stream, const char *program)
{
    fprintf(stream, "Usage: %s [--address IPv4] [--port 1-65535] [--once]\n", program);
}

static int parse_port(const char *text, uint16_t *port)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value == 0 || value > UINT16_MAX) {
        return -1;
    }

    *port = (uint16_t)value;
    return 0;
}

int main(int argc, char **argv)
{
    struct server_config config = {
        .address = "127.0.0.1",
        .port = 8080,
        .backlog = 128,
        .once = false,
    };
    struct server server = {.listener_fd = -1};

    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--help") == 0) {
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[index], "--once") == 0) {
            config.once = true;
            continue;
        }
        if (strcmp(argv[index], "--address") == 0 && index + 1 < argc) {
            config.address = argv[++index];
            continue;
        }
        if (strcmp(argv[index], "--port") == 0 && index + 1 < argc) {
            if (parse_port(argv[++index], &config.port) != 0) {
                fprintf(stderr, "invalid port: %s\n", argv[index]);
                return EXIT_FAILURE;
            }
            continue;
        }

        fprintf(stderr, "unknown or incomplete argument: %s\n", argv[index]);
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal(SIGPIPE)");
        return EXIT_FAILURE;
    }

    if (server_init(&server, &config) != 0) {
        return EXIT_FAILURE;
    }

    int result = server_run(&server);
    server_close(&server);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

