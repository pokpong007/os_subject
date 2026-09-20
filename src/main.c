#include "config.h"
#include "monitor.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *config_file = "config.txt";
    if (argc > 1) {
        config_file = argv[1];
    }

    Config config;
    if (load_config(config_file, &config) == -1) {
        printf("Cannot read config file: %s\n", config_file);
        return 1;
    }

    pid_t pid;
    if (read_pid(config.pid_file, &pid) == -1) {
        printf("Cannot read PID file: %s\n", config.pid_file);
        return 1;
    }

    printf("Monitoring PID %ld\n", (long)pid);

    while (1) {
        char state = '?';
        MonitorStatus status = check_process(
            pid, config.host, config.port, config.timeout, &state);

        if (status != MONITOR_OK) {
            printf("Failure: %s\n", monitor_status_text(status));
            return (int)status;
        }

        printf("PID %ld state %c server running\n", (long)pid, state);
        sleep((unsigned int)config.interval);
    }
}

