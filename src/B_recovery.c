#include "B_recovery.h"
#include "B_logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


// ============================================================
// Function Prototypes
// ============================================================

static pid_t start_server(const BConfig* config);

static int wait_for_process(pid_t pid);

static int wait_for_server(
    const char* host,
    int port
);

static int write_pid_file(
    const char* path,
    pid_t pid
);

static const char* failure_reason_to_string(
    FailureReason reason
);


// ============================================================
// Convert FailureReason to String
// ============================================================

static const char* failure_reason_to_string(
    FailureReason reason
)
{
    switch (reason) {

    case FAILURE_PROCESS_MISSING:
        return "PROCESS_MISSING";

    case FAILURE_PROCESS_STOPPED:
        return "PROCESS_STOPPED";

    case FAILURE_SERVER_UNRESPONSIVE:
        return "SERVER_UNRESPONSIVE";

    default:
        return "UNKNOWN";
    }
}


// ============================================================
// Recovery Interface
// ============================================================

RecoveryResult recover_process(
    const RecoveryRequest* request,
    const BConfig* config
)
{
    RecoveryResult result;

    result.success = 0;
    result.new_pid = -1;

    printf("Recovery started\n");
    printf("Old PID: %d\n", request->old_pid);


    // --------------------------------------------------------
    // Start Minecraft Server
    // --------------------------------------------------------

    pid_t new_pid = start_server(config);

    char command[1024];

    snprintf(
        command,
        sizeof(command),
        "java -Xmx%s -Xms%s -jar %s nogui",
        config->memory_max,
        config->memory_min,
        config->server_jar
    );

    if (new_pid < 0) {

        log_recovery(
            config,
            request->old_pid,
            failure_reason_to_string(request->reason),
            "Failed to start server",
            0,
            -1
        );

        printf("Failed to start server\n");

        return result;
    }

    printf("New PID: %d\n", new_pid);


    // --------------------------------------------------------
    // Check Process
    // --------------------------------------------------------

    if (!wait_for_process(new_pid)) {

        log_recovery(
            config,
            request->old_pid,
            failure_reason_to_string(request->reason),
            command,
            0,
            new_pid
        );

        printf("Server process failed to start\n");

        return result;
    }


    // --------------------------------------------------------
    // Check Minecraft Server Ready
    // --------------------------------------------------------

    if (!wait_for_server(
        config->host,
        config->port
    )) {

        log_recovery(
            config,
            request->old_pid,
            failure_reason_to_string(request->reason),
            command,
            0,
            new_pid
        );

        printf("Minecraft server failed to become ready\n");

        return result;
    }


    // --------------------------------------------------------
    // Write New PID
    // --------------------------------------------------------

    if (write_pid_file(
        config->pid_file,
        new_pid
    ) < 0) {

        log_recovery(
            config,
            request->old_pid,
            failure_reason_to_string(request->reason),
            command,
            0,
            new_pid
        );

        printf("Failed to write PID file\n");

        return result;
    }


    // --------------------------------------------------------
    // Log Successful Recovery
    // --------------------------------------------------------

    log_recovery(
        config,
        request->old_pid,
        failure_reason_to_string(request->reason),
        command,
        1,
        new_pid
    );


    // --------------------------------------------------------
    // Return Result to A
    // --------------------------------------------------------

    result.success = 1;
    result.new_pid = new_pid;

    printf("Recovery SUCCESS\n");
    printf("New PID: %d\n", new_pid);

    return result;
}


// ============================================================
// Start Minecraft Server
// ============================================================

static pid_t start_server(const BConfig* config)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {

        if (chdir(config->server_path) != 0) {
            perror("chdir");
            _exit(1);
        }

        char memory_max[64];
        char memory_min[64];
        char server_jar[256];

        snprintf(
            memory_max,
            sizeof(memory_max),
            "-Xmx%s",
            config->memory_max
        );

        snprintf(
            memory_min,
            sizeof(memory_min),
            "-Xms%s",
            config->memory_min
        );

        snprintf(
            server_jar,
            sizeof(server_jar),
            "%s",
            config->server_jar
        );

        char* args[] = {
            "java",
            memory_max,
            memory_min,
            "-jar",
            server_jar,
            "nogui",
            NULL
        };

        execvp("java", args);

        perror("execvp");
        _exit(1);
    }

    return pid;
}

// ============================================================
// Check if Process is Still Running
// ============================================================

static int wait_for_process(pid_t pid)
{
    for (int i = 0; i < 10; i++) {

        if (kill(pid, 0) == 0) {

            return 1;
        }

        sleep(1);
    }

    return 0;
}


// ============================================================
// Wait for Minecraft Server
// ============================================================

static int wait_for_server(
    const char* host,
    int port
)
{
    for (int i = 0; i < 60; i++) {

        int sock = socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

        if (sock < 0) {

            perror("socket");

            return 0;
        }


        struct sockaddr_in server;

        memset(
            &server,
            0,
            sizeof(server)
        );

        server.sin_family = AF_INET;
        server.sin_port = htons(port);


        if (inet_pton(
            AF_INET,
            host,
            &server.sin_addr
        ) <= 0) {

            close(sock);

            return 0;
        }


        if (connect(
            sock,
            (struct sockaddr*)&server,
            sizeof(server)
        ) == 0) {

            close(sock);

            printf(
                "Minecraft server is ready\n"
            );

            return 1;
        }


        close(sock);


        printf(
            "Waiting for Minecraft server... (%d/60)\n",
            i + 1
        );

        sleep(1);
    }


    printf(
        "Minecraft server did not become ready\n"
    );

    return 0;
}


// ============================================================
// Write New PID
// ============================================================

static int write_pid_file(
    const char* path,
    pid_t pid
)
{
    FILE* file = fopen(path, "w");


    if (file == NULL) {

        perror("fopen");

        return -1;
    }


    fprintf(
        file,
        "%d\n",
        pid
    );

    fclose(file);

    return 0;
}