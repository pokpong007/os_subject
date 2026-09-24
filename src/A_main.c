#include "A_config.h"
#include "A_monitor.h"
#include "B_config.h"
#include "B_recovery.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *config_file = "A_config.txt";
    const char *recovery_config_file = "Bconfig.ini";
    if (argc > 1) {
        config_file = argv[1];
    }
    if (argc > 2) {
        recovery_config_file = argv[2];
    }

    Config config;
    if (load_config(config_file, &config) == -1) {
        printf("Cannot read config file: %s\n", config_file);
        return 1;
    }

    BConfig recovery_config;
    if (load_b_config(recovery_config_file, &recovery_config) == -1) {
        printf("Cannot read recovery config file: %s\n", recovery_config_file);
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

            RecoveryRequest request;
            request.old_pid = pid;

            if (status == MONITOR_PROCESS_NOT_FOUND) {
                request.reason = FAILURE_PROCESS_MISSING;
            } else if (status == MONITOR_PROCESS_STOPPED) {
                request.reason = FAILURE_PROCESS_STOPPED;
            } else {
                request.reason = FAILURE_SERVER_UNRESPONSIVE;
            }

            RecoveryResult result = recover_process(&request, &recovery_config);
            if (!result.success) {
                printf("Recovery failed\n");
                return 1;
            }

            pid = result.new_pid;
            printf("Monitoring restarted PID %ld\n", (long)pid);
            continue;
        }

        printf("PID %ld state %c server running\n", (long)pid, state);
        sleep((unsigned int)config.interval);
    }
}
