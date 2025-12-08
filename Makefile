 # Simple Makefile for Flight Server
CXX = g++
CXXFLAGS = -std=c++14 -pthread -O2 -Wall -Wextra
LDFLAGS = -pthread

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = .

# Target executable
TARGET = flight_server

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/FlightServer.cpp
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) data logs

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET) *.log

# Run the server
run: all
	./$(TARGET)

# Quick test
test:
	@echo "Testing server (must be running on port 8080)..."
	@curl -s http://localhost:8080/api/health | head -1

.PHONY: all clean run test directories