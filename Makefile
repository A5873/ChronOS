##
## Makefile for MiniSched - A Minimalist Process Scheduler
##

# Compiler and tools
CC = gcc
RM = rm -f

# Source files and output files
SRCS = minisched.c main.c
OBJS = $(SRCS:.c=.o)
TARGET = minisched
TARGET_DEBUG = $(TARGET)_debug

# Test files
TEST_SRCS = minisched.c test_minisched.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = $(TARGET)_test
TEST_TARGET_DEBUG = $(TEST_TARGET)_debug

# Include directories
INCLUDES = -I.

# Compiler flags
# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -Werror: Treat warnings as errors
# -std=c11: Use C11 standard
# -pedantic: Issue warnings for strict ISO C compliance
COMMON_CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic $(INCLUDES)

# Debug flags
# -g: Add debugging information
# -O0: No optimization
# -DDEBUG: Define DEBUG macro
DEBUG_CFLAGS = $(COMMON_CFLAGS) -g -O0 -DDEBUG

# Release flags
# -O2: Moderate optimization
# -DNDEBUG: Define NDEBUG macro (disables assert)
RELEASE_CFLAGS = $(COMMON_CFLAGS) -O2 -DNDEBUG

# Linker flags
LDFLAGS =
LDLIBS = -lm

# Default target
all: release

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(TARGET_DEBUG)

# Release build
release: CFLAGS = $(RELEASE_CFLAGS)
release: $(TARGET)

# Test build (release)
test: CFLAGS = $(RELEASE_CFLAGS)
test: $(TEST_TARGET)

# Test build (debug)
test_debug: CFLAGS = $(DEBUG_CFLAGS)
test_debug: $(TEST_TARGET_DEBUG)

# Compiling the debug target
$(TARGET_DEBUG): $(OBJS)
	@echo "Building debug version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	@echo "Debug build complete: $@"

# Compiling the release target
$(TARGET): $(OBJS)
	@echo "Building release version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	@echo "Release build complete: $@"

# Rule for object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling the test target (release)
$(TEST_TARGET): $(TEST_OBJS)
	@echo "Building test version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(LDLIBS)
	@echo "Test build complete: $@"

# Compiling the test target (debug)
$(TEST_TARGET_DEBUG): $(TEST_OBJS)
	@echo "Building debug test version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(LDLIBS)
	@echo "Debug test build complete: $@"
# Generate dependencies automatically
depend: $(SRCS) $(TEST_SRCS)
	$(CC) -MM $(COMMON_CFLAGS) $(SRCS) $(TEST_SRCS) > .depend

# Include the dependency file if it exists
-include .depend

# Run the program (debug version)
run_debug: debug
	./$(TARGET_DEBUG)

# Run the program (release version)
run: release
	./$(TARGET)

# Run the tests (release version)
run_test: test
	@echo "Running test suite..."
	./$(TEST_TARGET)

# Run the tests (debug version)
run_test_debug: test_debug
	@echo "Running debug test suite..."
	./$(TEST_TARGET_DEBUG)

# Clean build files
clean:
	$(RM) $(OBJS) $(TEST_OBJS) $(TARGET) $(TARGET_DEBUG) $(TEST_TARGET) $(TEST_TARGET_DEBUG) .depend

# Clean everything including dependency files
distclean: clean
	$(RM) *~ core

# Targets that don't correspond to files
.PHONY: all debug release test test_debug clean distclean depend run run_debug run_test run_test_debug

# Help target
help:
	@echo "MiniSched Makefile"
	@echo "================="
	@echo "Available targets:"
	@echo "  all        - Build release version (default)"
	@echo "  debug      - Build debug version"
	@echo "  release    - Build release version"
	@echo "  run        - Build and run release version"
	@echo "  run_debug  - Build and run debug version"
	@echo "  test       - Build test suite (release)"
	@echo "  test_debug - Build test suite (debug)"
	@echo "  run_test   - Build and run test suite (release)"
	@echo "  run_test_debug - Build and run test suite (debug)"
	@echo "  clean      - Remove object files and binaries"
	@echo "  distclean  - Remove all generated files"
	@echo "  depend     - Generate dependencies"
	@echo "  help       - Display this help message"
