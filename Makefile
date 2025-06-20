SRC_DIR := src
HEADER_DIR := headers
BIN_DIR := bin
BUILD_DIR := build
DEP_DIR := deps

CC := gcc
CFLAGS := -Wall -Wextra -I$(HEADER_DIR)
CPPFLAGS := -I$(HEADER_DIR)
GPFLAGS :=

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

GPERF_SRCS := $(shell find $(SRC_DIR) -name '*.gperf')
GPERF_GEN_C := $(GPERF_SRCS:$(SRC_DIR)/%.gperf=$(BUILD_DIR)/%.gperf.c)
GPERF_OBJS := $(GPERF_GEN_C:$(BUILD_DIR)/%.gperf.c=$(BUILD_DIR)/%.gperf.o)

ALL_OBJS := $(OBJS) $(GPERF_OBJS) 

$(BIN_DIR)/pras: $(ALL_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(CFLAGS) -o $@

$(BIN_DIR)/pras-debug: $(ALL_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(CFLAGS) -o $@ -g

$(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -o $@ -c

# Generate C from gperf, handle subdirs
$(BUILD_DIR)/%.gperf.c: $(SRC_DIR)/%.gperf
	@mkdir -p $(dir $@)
	@funcname=$(shell echo $* | sed 's/[^a-zA-Z0-9_]/_/g'); \
	gperf -a -N lookup_$$funcname -L ANSI-C -t $< > $@

# Compile generated gperf C
$(BUILD_DIR)/%.gperf.o: $(BUILD_DIR)/%.gperf.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	@set -e; mkdir -p $(DEP_DIR); rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DEP_DIR)/%.gperf.d: $(BUILD_DIR)/%.gperf.c
	@set -e; mkdir -p $(DEP_DIR); rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\).gperf\.o[ :]*,$(BUILD_DIR)/\1.gperf.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	rm -r $(DEP_DIR)

-include $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)
-include $(GPERF_GEN_C:$(BUILD_DIR)/%.gperf.c=$(DEP_DIR)/%.gperf.d)
