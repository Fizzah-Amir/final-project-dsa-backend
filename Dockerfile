 FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    build-essential \
    cmake \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# First, check what files we have
RUN echo "=== Project Structure ===" && \
    find . -name "*.cpp" -o -name "*.h" | head -20 && \
    echo "=== Main files ===" && \
    ls -la *.cpp *.h 2>/dev/null || true

# Check if main.cpp exists in current directory
RUN if [ ! -f "main.cpp" ]; then \
    echo "⚠️  main.cpp not found in root. Searching..."; \
    find . -name "main.cpp" -type f; \
    fi

# Compile with your main.cpp and FlightServer.cpp
RUN echo "=== Compiling Flight Management System ===" && \
    # Find main.cpp wherever it is
    MAIN_FILE=$(find . -name "main.cpp" -type f | head -1) && \
    FLIGHT_SERVER_FILE=$(find . -name "FlightServer.cpp" -type f | head -1) && \
    echo "Using main file: $MAIN_FILE" && \
    echo "Using FlightServer file: $FLIGHT_SERVER_FILE" && \
    # Compile
    g++ -o server \
        "$MAIN_FILE" \
        "$FLIGHT_SERVER_FILE" \
        -std=c++17 \
        -pthread \
        -I/app \
        -I. \
        -I./include \
        $(find . -name "*.cpp" -type f ! -name "main.cpp" ! -name "FlightServer.cpp" | tr '\n' ' ') \
        2>&1 | tee compile.log

# Alternative: If you want to compile specific files (adjust paths):
RUN echo "=== Alternative compilation ===" && \
    g++ -o server \
        main.cpp \
        FlightServer.cpp \
        -std=c++17 \
        -pthread \
        -I. \
        -I./include \
        2>&1 | tee compile.log || \
    echo "First method failed, trying different approach..."

# Check if compilation succeeded
RUN if [ -f "/app/server" ]; then \
        echo "✅ Compilation SUCCESS!" && \
        echo "📦 Executable details:" && \
        file server && \
        ls -lh server; \
    else \
        echo "❌ Compilation FAILED!" && \
        echo "📋 Last 50 lines of compilation log:" && \
        tail -50 compile.log; \
        exit 1; \
    fi

# Test run (quick check)
RUN echo "=== Quick test ===" && \
    timeout 3s ./server 9999 2>&1 | head -10 || echo "Test completed"

EXPOSE 8080

# Railway provides PORT environment variable - use it!
CMD ./server ${PORT:-8080}