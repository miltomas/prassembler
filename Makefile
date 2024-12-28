SRC_DIR := src
HEADER_DIR := headers
BIN_DIR := bin
BUILD_DIR := build
DEP_DIR := deps

CC := gcc
CFLAGS := -Wall -Wextra -I$(HEADER_DIR)
CPPFLAGS := -I$(HEADER_DIR)

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BIN_DIR)/pras: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(CFLAGS) -o $@

$(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $< $(CFLAGS) -o $@ -c

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	@set -e; mkdir -p $(DEP_DIR); rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	rm -r $(DEP_DIR)

include $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)
