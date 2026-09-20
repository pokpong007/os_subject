#include "A_monitor.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int read_pid(const char *filename, pid_t *pid)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    long value;
    int result = fscanf(file, "%ld", &value);
    fclose(file);

    if (result != 1 || value <= 0) {
        return -1;
    }

    *pid = (pid_t)value;
    return 0;
}

int read_process_state(pid_t pid, char *state)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/status", (long)pid);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "State:\t%c", state) == 1) {
            found = 1;
            break;
        }
    }

    fclose(file);
    return found ? 0 : -1;
}

int check_server(const char *host, int port, int timeout)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        return -1;
    }

    struct timeval time_limit;
    time_limit.tv_sec = timeout;
    time_limit.tv_usec = 0;

    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &time_limit, sizeof(time_limit));
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time_limit, sizeof(time_limit));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);

    if (inet_pton(AF_INET, host, &address.sin_addr) != 1) {
        close(socket_fd);
        return -1;
    }

    int result = connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
    close(socket_fd);
    return result;
}

MonitorStatus check_process(pid_t pid, const char *host, int port, int timeout,
                            char *state)
{
    if (kill(pid, 0) == -1 && errno == ESRCH) {
        return MONITOR_PROCESS_NOT_FOUND;
    }

    if (read_process_state(pid, state) == -1) {
        return MONITOR_PROCESS_NOT_FOUND;
    }

    if (*state == 'Z' || *state == 'T') {
        return MONITOR_PROCESS_STOPPED;
    }

    if (check_server(host, port, timeout) == -1) {
        return MONITOR_SERVER_NOT_RESPONDING;
    }

    return MONITOR_OK;
}

const char *monitor_status_text(MonitorStatus status)
{
    switch (status) {
    case MONITOR_OK:
        return "running";
    case MONITOR_PROCESS_NOT_FOUND:
        return "process not found";
    case MONITOR_PROCESS_STOPPED:
        return "process stopped";
    case MONITOR_SERVER_NOT_RESPONDING:
        return "server not responding";
    }

    return "unknown";
}
