CC = gcc
CFLAGS = -O1 -mavx2 -g -fopenmp
SRC_DIR = .
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.c)
TARGETS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%,$(SOURCES))

all: $(BUILD_DIR) $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
