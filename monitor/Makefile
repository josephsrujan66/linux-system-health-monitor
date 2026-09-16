CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -pthread

TARGET = system_monitor

SRC = src/main.c \
      src/system_info.c \
      src/memory_monitor.c \
      src/cpu_monitor.c \
      src/disk_monitor.c \
      src/network_monitor.c \
      src/monitor_engine.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
