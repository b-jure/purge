CC = gcc
CFLAGS = -Wl,-z,relro,-z,now -O2 -Wall -Wextra -std=c99
DCFLAGS = -ggdb -Wall -Wextra -std=c99
BUILD_DIR = build
BIN := $(BUILD_DIR)/purge
DBIN := $(BUILD_DIR)/dpurge

all: $(BUILD_DIR) $(BIN)

$(BIN): $(BUILD_DIR)/purge.o
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/purge.o: purge.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: $(BUILD_DIR) $(DBIN)

$(DBIN): $(BUILD_DIR)/dpurge.o
	$(CC) $(DCFLAGS) $< -o $@

$(BUILD_DIR)/dpurge.o: purge.c
	$(CC) $(DCFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p build

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
