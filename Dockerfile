FROM gcc:12.2.0

WORKDIR /app
COPY . .

RUN echo "=== Compiling Flight Server ===" && \
    g++ -o server \
        src/main.cpp \
        src/FlightServer.cpp \
        -std=c++17 \
        -pthread \
        -I./include \
        2>&1 | tee compile.log

RUN if [ -f server ]; then \
        echo "✅ Server compiled successfully!" && \
        ls -lh server; \
    else \
        echo "❌ Compilation failed!" && \
        cat compile.log; \
        exit 1; \
    fi

EXPOSE 8080
CMD ./server ${PORT:-8080}
