CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude

all: monitor

monitor: src/main.c src/config.c src/monitor.c
	$(CC) $(CFLAGS) src/main.c src/config.c src/monitor.c -o monitor

clean:
	rm -f monitor

