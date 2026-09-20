#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h>

typedef enum {
    MONITOR_OK,
    MONITOR_PROCESS_NOT_FOUND,
    MONITOR_PROCESS_STOPPED,
    MONITOR_SERVER_NOT_RESPONDING
} MonitorStatus;

int read_pid(const char *filename, pid_t *pid);
int read_process_state(pid_t pid, char *state);
int check_server(const char *host, int port, int timeout);
MonitorStatus check_process(pid_t pid, const char *host, int port, int timeout,
                            char *state);
const char *monitor_status_text(MonitorStatus status);

#endif

