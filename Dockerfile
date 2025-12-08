FROM gcc:12.2.0
WORKDIR /app
COPY . .
RUN g++ -o server src/main.cpp src/FlightServer.cpp -std=c++17 -pthread -I./include
EXPOSE 8080
CMD ./server ${PORT:-8080}
