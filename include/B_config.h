#ifndef B_CONFIG_H
#define B_CONFIG_H

typedef struct {

    char pid_file[256];

    char host[64];
    int port;

    char server_path[512];
    char server_jar[256];

    char log_file[256];

    char memory_min[32];
    char memory_max[32];

} BConfig;

int load_b_config(
    const char* filename,
    BConfig* config
);

#endif