 #!/bin/bash

echo "=================================================="
echo "   Starting Flight Management System Server"
echo "=================================================="

# Check if port 8080 is in use
if lsof -Pi :8080 -sTCP:LISTEN -t >/dev/null ; then
    echo "⚠️  Port 8080 is already in use!"
    echo "   Killing existing process..."
    kill -9 $(lsof -t -i:8080) 2>/dev/null
    sleep 2
fi

# Build if needed
if [ ! -f "flight_server" ]; then
    echo "🔨 Building server..."
    make clean
    make
    if [ $? -ne 0 ]; then
        echo "❌ Build failed!"
        exit 1
    fi
fi

# Create necessary directories
mkdir -p logs data

echo "🚀 Starting server on port 8080..."
echo "📝 Logs: logs/server_$(date +%Y%m%d_%H%M%S).log"
echo "💡 Press Ctrl+C to stop"
echo ""

# Run the server
./flight_server