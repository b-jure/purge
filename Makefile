CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c99
DFLAGS = -ggdb -Wall -Wextra -std=c99
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
	$(CC) $(DFLAGS) $< -o $@

$(BUILD_DIR)/dpurge.o: purge.c
	$(CC) $(DFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p build

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
