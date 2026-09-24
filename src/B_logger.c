#include "B_logger.h"

#include <stdio.h>
#include <time.h>

void log_recovery(
    const BConfig* config,
    int old_pid,
    const char* reason,
    const char* command,
    int success,
    int new_pid
)
{
    FILE* file = fopen(config->log_file, "a");

    if (file == NULL) {
        perror("fopen");
        return;
    }

    time_t now = time(NULL);

    struct tm* local_time = localtime(&now);

    char time_buffer[32];

    strftime(
        time_buffer,
        sizeof(time_buffer),
        "%Y-%m-%d %H:%M:%S",
        local_time
    );

    fprintf(
        file,
        "%s | old_pid=%d | reason=%s | command=\"%s\" | result=%s | new_pid=%d\n",
        time_buffer,
        old_pid,
        reason,
        command,
        success ? "SUCCESS" : "FAILED",
        new_pid
    );

    fclose(file);
}