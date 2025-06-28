# Makefile for GTU-C312 Simulator
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = Simulate
SOURCES = main.cpp

# Build the simulator
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Clean build files
clean:
	rm -f $(TARGET)

# Test with the sample program
test: $(TARGET)
	./$(TARGET) sample_program.txt -D 1

# Test with different debug modes
test-debug0: $(TARGET)
	./$(TARGET) sample_program.txt -D 0

test-debug1: $(TARGET)
	./$(TARGET) sample_program.txt -D 1

test-debug2: $(TARGET)
	./$(TARGET) sample_program.txt -D 2

test-debug3: $(TARGET)
	./$(TARGET) sample_program.txt -D 3

# Help
help:
	@echo "Available targets:"
	@echo "  $(TARGET)    - Build the simulator"
	@echo "  clean       - Remove build files"
	@echo "  test        - Run test with debug mode 1"
	@echo "  test-debug0 - Run test with debug mode 0"
	@echo "  test-debug1 - Run test with debug mode 1"
	@echo "  test-debug2 - Run test with debug mode 2"
	@echo "  test-debug3 - Run test with debug mode 3"
	@echo "  help        - Show this help"

.PHONY: clean test test-debug0 test-debug1 test-debug2 test-debug3 help