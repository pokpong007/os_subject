#ifndef B_LOGGER_H
#define B_LOGGER_H

#include "B_config.h"

void log_recovery(
    const BConfig* config,
    int old_pid,
    const char* reason,
    const char* command,
    int success,
    int new_pid
);

#endif