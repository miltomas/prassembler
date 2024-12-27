SRC_DIR := src
HEADER_DIR := headers
BIN_DIR := bin
BUILD_DIR := build

CC := gcc
CFLAGS := -Wall -Wextra -I$(HEADER_DIR)

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BIN_DIR)/pras: $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $^ $(CFLAGS) -o $@

$(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)	
	$(CC) $^ $(CFLAGS) -o $@ -c

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
