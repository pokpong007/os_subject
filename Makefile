CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude

all: A_monitor

A_monitor: src/A_main.c src/A_config.c src/A_monitor.c
	$(CC) $(CFLAGS) src/A_main.c src/A_config.c src/A_monitor.c -o A_monitor

clean:
	rm -f A_monitor
