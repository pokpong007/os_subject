#ifndef B_RECOVERY_H
#define B_RECOVERY_H

#include <sys/types.h>

#include "B_config.h"

typedef enum {
    FAILURE_PROCESS_MISSING,
    FAILURE_PROCESS_STOPPED,
    FAILURE_SERVER_UNRESPONSIVE
} FailureReason;

typedef struct {
    FailureReason reason;
    pid_t old_pid;
} RecoveryRequest;

typedef struct {
    int success;
    pid_t new_pid;
} RecoveryResult;

RecoveryResult recover_process(
    const RecoveryRequest* request,
    const BConfig* config
);

#endif