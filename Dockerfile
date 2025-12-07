FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    g++ \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Try multiple possible main files
RUN if [ -f "source/data_structures/functionalities/test_complete_system.cpp" ]; then \
        echo "Compiling test_complete_system.cpp..."; \
        g++ -o server source/data_structures/functionalities/test_complete_system.cpp -std=c++11 -pthread -I.; \
    elif [ -f "server.cpp" ]; then \
        echo "Compiling server.cpp..."; \
        g++ -o server server.cpp -std=c++11 -pthread; \
    else \
        MAIN_FILE=$(find . -name "*.cpp" -type f | head -1); \
        echo "Compiling first found file: $MAIN_FILE"; \
        g++ -o server "$MAIN_FILE" -std=c++11 -pthread -I.; \
    fi

EXPOSE 8080
CMD ["./server", "8080"]
