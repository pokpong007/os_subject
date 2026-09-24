#include "B_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void remove_newline(char* str)
{
    str[strcspn(str, "\r\n")] = '\0';
}

static void set_config_value(
    BConfig* config,
    const char* key,
    const char* value
)
{
    if (strcmp(key, "PID_FILE") == 0) {

        snprintf(
            config->pid_file,
            sizeof(config->pid_file),
            "%s",
            value
        );

    }
    else if (strcmp(key, "HOST") == 0) {

        snprintf(
            config->host,
            sizeof(config->host),
            "%s",
            value
        );

    }
    else if (strcmp(key, "PORT") == 0) {

        config->port = atoi(value);

    }
    else if (strcmp(key, "SERVER_PATH") == 0) {

        snprintf(
            config->server_path,
            sizeof(config->server_path),
            "%s",
            value
        );

    }
    else if (strcmp(key, "SERVER_JAR") == 0) {

        snprintf(
            config->server_jar,
            sizeof(config->server_jar),
            "%s",
            value
        );

    }
    else if (strcmp(key, "LOG_FILE") == 0) {

        snprintf(
            config->log_file,
            sizeof(config->log_file),
            "%s",
            value
        );

    }
    else if (strcmp(key, "MEMORY_MIN") == 0) {

        snprintf(
            config->memory_min,
            sizeof(config->memory_min),
            "%s",
            value
        );

    }
    else if (strcmp(key, "MEMORY_MAX") == 0) {

        snprintf(
            config->memory_max,
            sizeof(config->memory_max),
            "%s",
            value
        );
    }
}

int load_b_config(
    const char* filename,
    BConfig* config
)
{
    memset(config, 0, sizeof(BConfig));

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen config");
        return -1;
    }

    char line[1024];

    while (fgets(line, sizeof(line), file) != NULL) {

        remove_newline(line);

        if (line[0] == '\0') {
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        char* equal = strchr(line, '=');

        if (equal == NULL) {
            continue;
        }

        *equal = '\0';

        const char* key = line;
        const char* value = equal + 1;

        set_config_value(
            config,
            key,
            value
        );
    }

    fclose(file);

    return 0;
}