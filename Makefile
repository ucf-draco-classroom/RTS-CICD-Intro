# Makefile -- autogen -mb
CC       := gcc
CFLAGS   := -std=c11 -O2 -Wall -Wextra -pthread
LDFLAGS  := -pthread
BUILD    := build
TARGET   := $(BUILD)/rt_hello
SRC      := src/main.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Run the program and fail if the PASS banner isn't seen
test: $(TARGET)
	./$(TARGET) | tee $(BUILD)/output.txt
	@grep -q "SELF_TEST_PASS" $(BUILD)/output.txt

clean:
	rm -rf $(BUILD)
