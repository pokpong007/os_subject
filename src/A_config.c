#include "A_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_config(const char *filename, Config *config)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    strcpy(config->pid_file, "server.pid");
    strcpy(config->host, "127.0.0.1");
    config->port = 25565;
    config->interval = 5;
    config->timeout = 3;

    char line[320];
    char key[64];
    char value[256];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%63[^=]=%255s", key, value) != 2) {
            continue;
        }

        if (strcmp(key, "pid_file") == 0) {
            snprintf(config->pid_file, sizeof(config->pid_file), "%s", value);
        } else if (strcmp(key, "host") == 0) {
            snprintf(config->host, sizeof(config->host), "%s", value);
        } else if (strcmp(key, "port") == 0) {
            config->port = atoi(value);
        } else if (strcmp(key, "interval") == 0) {
            config->interval = atoi(value);
        } else if (strcmp(key, "timeout") == 0) {
            config->timeout = atoi(value);
        }
    }

    fclose(file);

    if (config->port <= 0 || config->port > 65535 ||
        config->interval <= 0 || config->timeout <= 0) {
        return -1;
    }

    return 0;
}
