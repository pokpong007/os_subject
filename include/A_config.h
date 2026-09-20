#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char pid_file[256];
    char host[64];
    int port;
    int interval;
    int timeout;
} Config;

int load_config(const char *filename, Config *config);

#endif
