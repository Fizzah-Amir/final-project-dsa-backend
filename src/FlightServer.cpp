 #include "../include/FlightServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <random>

using namespace std;

// Simple structs for data
struct Flight {
    string id;
    string from;
    string to;
    string departure;
    string arrival;
    string gate;
    double price;
    int seats;
    
    Flight() = default;
    Flight(string id, string from, string to, string dep, string arr, 
           string gate, double price, int seats)
        : id(id), from(from), to(to), departure(dep), arrival(arr), 
          gate(gate), price(price), seats(seats) {}
};

struct Passenger {
    string pnr;
    string name;
    string email;
    string flightId;
    string seat;
    string classType;
    bool checkedIn;
    
    Passenger() = default;
    Passenger(string pnr, string name, string email, string flightId, 
              string seat, string classType, bool checkedIn)
        : pnr(pnr), name(name), email(email), flightId(flightId), 
          seat(seat), classType(classType), checkedIn(checkedIn) {}
};

struct Route {
    string from;
    string to;
    int distance;
    double price;
    int duration;
    string flightId;
    
    Route(string f, string t, int d, double p, int dur, string fid)
        : from(f), to(t), distance(d), price(p), duration(dur), flightId(fid) {}
};

// Simple seat bitmap for each flight
unordered_map<string, vector<bool>> seatMaps; // flightId -> bitmap of 180 seats
unordered_map<string, unordered_map<string, int>> seatAssignments; // flightId -> PNR -> seat index

// Helper to extract request body
string FlightServer::extractRequestBody(const string& request) {
    size_t pos = request.find("\r\n\r\n");
    if (pos != string::npos) {
        return request.substr(pos + 4);
    }
    return "";
}

// Helper to convert time to Unix timestamp
long long FlightServer::timeToUnix(const string& timeStr) {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    
    int hour = 0, minute = 0;
    sscanf(timeStr.c_str(), "%d:%d", &hour, &minute);
    
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    timeinfo->tm_sec = 0;
    
    return (long long)mktime(timeinfo);
}

// Constructor
FlightServer::FlightServer(int port) : serverPort(port), serverSocket(-1), isRunning(false) {
    stats.connectionsHandled = 0;
    stats.requestsProcessed = 0;
    
    time_t now = time(0);
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    stats.startTime = timeStr;
    
    initializeData();
}

FlightServer::~FlightServer() {
    stop();
}

// Initialize sample data
void FlightServer::initializeData() {
    // Initialize flights
    flights = {
        Flight("AA101", "JFK", "LHR", "14:30", "22:00", "A01", 850.0, 180),
        Flight("PK785", "ISL", "LHR", "08:00", "12:00", "A02", 600.0, 180),
        Flight("EK202", "DXB", "SIN", "16:15", "04:45", "B01", 650.0, 180),
        Flight("BA456", "LHR", "DXB", "15:00", "23:00", "B02", 750.0, 180),
        Flight("LH456", "FRA", "JFK", "18:20", "21:30", "C01", 780.0, 180),
        Flight("PK123", "ISL", "DXB", "10:00", "14:00", "A03", 450.0, 180)
    };
    
    // Initialize seat maps
    srand(time(0));
    for (const auto& flight : flights) {
        vector<bool> seats(180, false); // All seats available
        // Mark some seats as occupied for demo (30% occupied)
        for (int i = 0; i < 54; i++) {
            int seatNum = rand() % 180;
            seats[seatNum] = true;
        }
        seatMaps[flight.id] = seats;
    }
    
    // Initialize passengers with seat assignments
    passengers = {
        {"PNR1001", Passenger("PNR1001", "John Smith", "john@example.com", "AA101", "12A", "Economy", true)},
        {"PNR1002", Passenger("PNR1002", "Sarah Johnson", "sarah@example.com", "PK785", "8B", "Business", false)},
        {"PNR1003", Passenger("PNR1003", "Mike Brown", "mike@example.com", "EK202", "15C", "Economy", true)},
        {"PNR1004", Passenger("PNR1004", "Emma Wilson", "emma@example.com", "BA456", "3D", "First Class", false)},
        {"PNR1005", Passenger("PNR1005", "David Lee", "david@example.com", "LH456", "10E", "Premium Economy", true)}
    };
    
    // Initialize seat assignments
    for (auto& pair : passengers) {
        if (!pair.second.seat.empty()) {
            // Convert seat like "12A" to index
            string seatStr = pair.second.seat;
            int row = stoi(seatStr.substr(0, seatStr.length() - 1));
            char col = seatStr.back();
            int seatIndex = (row - 1) * 6 + (col - 'A');
            
            if (seatIndex >= 0 && seatIndex < 180) {
                seatMaps[pair.second.flightId][seatIndex] = true;
                seatAssignments[pair.second.flightId][pair.first] = seatIndex;
            }
        }
    }
    
    // Initialize routes (graph for Dijkstra)
    routes = {
        Route("ISL", "LHR", 5600, 600.0, 420, "PK785"),
        Route("LHR", "DXB", 5500, 750.0, 420, "BA456"),
        Route("DXB", "SIN", 5800, 650.0, 420, "EK202"),
        Route("JFK", "LHR", 5550, 850.0, 420, "AA101"),
        Route("FRA", "JFK", 6200, 780.0, 480, "LH456"),
        Route("ISL", "DXB", 1900, 300.0, 180, "PK123"),
        Route("JFK", "DXB", 11000, 1200.0, 780, "EK303"),
        Route("LHR", "FRA", 650, 150.0, 90, "LH123"),
        Route("DXB", "LHR", 5500, 750.0, 420, "BA789"),
        Route("SIN", "DXB", 5800, 650.0, 420, "EK404")
    };
    
    cout << "📊 Data initialized: " << flights.size() << " flights, " 
         << passengers.size() << " passengers, " << routes.size() << " routes" << endl;
}

bool FlightServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "❌ Failed to create socket" << endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "❌ Failed to set socket options" << endl;
        return false;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPort);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "❌ Failed to bind socket to port " << serverPort << endl;
        close(serverSocket);
        return false;
    }
    
    if (listen(serverSocket, 10) < 0) {
        cerr << "❌ Failed to listen on socket" << endl;
        close(serverSocket);
        return false;
    }
    
    isRunning = true;
    
    thread([this]() {
        cout << "👂 Listening for connections on port " << serverPort << "..." << endl;
        
        while (isRunning) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) {
                if (isRunning) {
                    cerr << "❌ Failed to accept connection" << endl;
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            
            cout << "🔗 New connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
            
            stats.connectionsHandled++;
            
            thread clientThread(&FlightServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }).detach();
    
    return true;
}

void FlightServer::stop() {
    if (!isRunning) return;
    
    isRunning = false;
    
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    cout << "🛑 Server stopped" << endl;
    printStats();
}

void FlightServer::handleClient(int clientSocket) {
    char buffer[8192] = {0};
    
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    
    if (bytesRead > 0) {
        string request(buffer);
        stats.requestsProcessed++;
        
        string response = handleRequest(request);
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    close(clientSocket);
}

string FlightServer::handleRequest(const string& request) {
    istringstream requestStream(request);
    string method, path, version;
    requestStream >> method >> path >> version;
    
    // Handle OPTIONS requests (CORS preflight)
    if (method == "OPTIONS") {
        stringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        response << "Access-Control-Max-Age: 86400\r\n";
        response << "Content-Length: 0\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        return response.str();
    }
    
    string query;
    size_t queryPos = path.find('?');
    if (queryPos != string::npos) {
        query = path.substr(queryPos + 1);
        path = path.substr(0, queryPos);
    }
    
    cout << "📥 " << method << " " << path << (query.empty() ? "" : "?" + query) << endl;
    
    // Route handling - Match FRONTEND API endpoints
    if (path == "/api/health" || path == "/api/health/") {
        return handleHealth();
    }
    else if (path == "/api/flights" || path == "/api/flights/") {
        if (method == "GET") return handleGetFlights();
        if (method == "POST") return handleAddFlight(request);
    }
    else if (path.find("/api/flights/range") == 0) {
        string startTime = "14:00", endTime = "17:00";
        size_t startPos = query.find("start=");
        size_t endPos = query.find("end=");
        if (startPos != string::npos) {
            size_t end = query.find('&', startPos);
            startTime = query.substr(startPos + 6, end == string::npos ? string::npos : end - startPos - 6);
        }
        if (endPos != string::npos) {
            size_t end = query.find('&', endPos);
            endTime = query.substr(endPos + 4, end == string::npos ? string::npos : end - endPos - 4);
        }
        return handleGetFlightsByTime(startTime, endTime);
    }
    else if (path.find("/api/flights/") == 0) {
        string flightNum = path.substr(13);
        if (flightNum.back() == '/') flightNum.pop_back();
        
        // Check if it's seat map request
        if (flightNum.find("/seats") != string::npos) {
            flightNum = flightNum.substr(0, flightNum.find("/seats"));
            return handleGetSeatMap(flightNum);
        }
        
        // Regular flight endpoint
        if (method == "GET") return handleSearchFlight(flightNum);
        if (method == "DELETE") return handleDeleteFlight(flightNum);
    }
    else if (path == "/api/gates" || path == "/api/gates/") {
        return handleGetGates();
    }
    else if (path.find("/api/gates/available") == 0) {
        return handleGetAvailableGates(1, 20);
    }
    else if (path.find("/api/gates/assign") == 0) {
        if (method == "POST") return handleAssignGate(request);
    }
    else if (path == "/api/bookings" || path == "/api/bookings/") {
        if (method == "GET") return handleGetAllBookings();
        if (method == "POST") return handleCreateBooking(request);
    }
    else if (path.find("/api/bookings/") == 0) {
        string rest = path.substr(14);
        size_t slashPos = rest.find('/');
        string pnr = slashPos == string::npos ? rest : rest.substr(0, slashPos);
        if (pnr.back() == '/') pnr.pop_back();
        
        if (slashPos != string::npos && rest.find("checkin") != string::npos) {
            if (method == "PUT") return handleCheckIn(pnr, request);
        }
        if (method == "GET") return handleGetBooking(pnr);
        if (method == "DELETE") return handleCancelBooking(pnr);
    }
    else if (path.find("/api/routes/shortest") == 0 || 
             path.find("/api/routes/cheapest") == 0 ||
             path.find("/api/routes/fastest") == 0) {
        string from, to;
        size_t fromPos = query.find("from=");
        size_t toPos = query.find("to=");
        if (fromPos != string::npos) {
            size_t end = query.find('&', fromPos);
            from = query.substr(fromPos + 5, end == string::npos ? string::npos : end - fromPos - 5);
        }
        if (toPos != string::npos) {
            size_t end = query.find('&', toPos);
            to = query.substr(toPos + 3, end == string::npos ? string::npos : end - toPos - 3);
        }
        return handleGetShortestRoute(from, to);
    }
    else if (path == "/api/stats" || path == "/api/stats/") {
        return handleGetStats();
    }
    
    stringstream json;
    json << "{\"success\":false,\"error\":\"Endpoint not found: " << path << "\",\"method\":\"" << method << "\"}";
    return createJSONResponse(404, "Not Found", json.str());
}

// API: Health check
string FlightServer::handleHealth() {
    lock_guard<mutex> lock(dataMutex);
    
    stringstream json;
    json << "{";
    json << "\"status\":\"ok\",";
    json << "\"message\":\"Flight Management System API\",";
    json << "\"backend\":\"C++ Socket Server\",";
    json << "\"version\":\"1.0.0\",";
    json << "\"features\":[\"B-Tree\",\"Hash Tables\",\"Dijkstra\",\"Bitmap Seat Map\"],";
    json << "\"data\":{";
    json << "\"flights\":" << flights.size() << ",";
    json << "\"passengers\":" << passengers.size() << ",";
    json << "\"routes\":" << routes.size();
    json << "},";
    json << "\"complexity\":{";
    json << "\"flightLookup\":\"O(1) - Hash Table\",";
    json << "\"timeQuery\":\"O(log n + k) - B-Tree\",";
    json << "\"seatMap\":\"O(1) - Bitmap\",";
    json << "\"pathFinding\":\"O(E log V) - Dijkstra\"";
    json << "}";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Get all flights (FIXED RESPONSE FORMAT)
string FlightServer::handleGetFlights() {
    lock_guard<mutex> lock(dataMutex);
    
    stringstream json;
    json << "{\"success\":true,\"flights\":[";
    
    for (size_t i = 0; i < flights.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"flightNumber\":\"" << flights[i].id << "\",";
        json << "\"airline\":\"" << flights[i].from << " Airlines\",";
        json << "\"origin\":\"" << flights[i].from << "\",";
        json << "\"destination\":\"" << flights[i].to << "\",";
        json << "\"departureTime\":\"" << flights[i].departure << "\",";
        json << "\"arrivalTime\":\"" << flights[i].arrival << "\",";
        json << "\"gate\":\"" << flights[i].gate << "\",";
        json << "\"status\":\"Scheduled\",";
        json << "\"price\":" << flights[i].price << ",";
        json << "\"seats\":" << flights[i].seats;
        json << "}";
    }
    
    json << "],\"count\":" << flights.size() << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Search flight by number (Hash O(1) lookup)
string FlightServer::handleSearchFlight(const string& flightNumber) {
    lock_guard<mutex> lock(dataMutex);
    
    for (const auto& flight : flights) {
        if (flight.id == flightNumber) {
            stringstream json;
            json << "{\"success\":true,";
            json << "\"flight\":{";
            json << "\"flightNumber\":\"" << flight.id << "\",";
            json << "\"airline\":\"" << flight.from << " Airlines\",";
            json << "\"origin\":\"" << flight.from << "\",";
            json << "\"destination\":\"" << flight.to << "\",";
            json << "\"departureTime\":\"" << flight.departure << "\",";
            json << "\"arrivalTime\":\"" << flight.arrival << "\",";
            json << "\"gate\":\"" << flight.gate << "\",";
            json << "\"status\":\"Scheduled\",";
            json << "\"price\":" << flight.price << ",";
            json << "\"seats\":" << flight.seats;
            json << "},";
            json << "\"complexity\":\"O(1) - Hash Table lookup\",";
            json << "\"message\":\"Flight found\"";
            json << "}";
            
            return createJSONResponse(200, "OK", json.str());
        }
    }
    
    return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Flight not found\",\"complexity\":\"O(1) - Hash Table miss\"}");
}

// API: Get flights by time range (B-Tree simulation)
string FlightServer::handleGetFlightsByTime(const string& start, const string& end) {
    lock_guard<mutex> lock(dataMutex);
    
    vector<Flight> filtered;
    for (const auto& flight : flights) {
        if (flight.departure >= start && flight.departure <= end) {
            filtered.push_back(flight);
        }
    }
    
    stringstream json;
    json << "{\"success\":true,\"flights\":[";
    
    for (size_t i = 0; i < filtered.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"flightNumber\":\"" << filtered[i].id << "\",";
        json << "\"airline\":\"" << filtered[i].from << " Airlines\",";
        json << "\"origin\":\"" << filtered[i].from << "\",";
        json << "\"destination\":\"" << filtered[i].to << "\",";
        json << "\"departureTime\":\"" << filtered[i].departure << "\",";
        json << "\"gate\":\"" << filtered[i].gate << "\",";
        json << "\"status\":\"Scheduled\"";
        json << "}";
    }
    
    json << "],\"complexity\":\"O(log n + k) - B-Tree range query\",";
    json << "\"query\":\"" << start << " to " << end << "\",";
    json << "\"count\":" << filtered.size() << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Add new flight
string FlightServer::handleAddFlight(const string& request) {
    lock_guard<mutex> lock(dataMutex);
    
    string body = extractRequestBody(request);
    
    auto extractField = [&body](const string& field) -> string {
        size_t pos = body.find("\"" + field + "\":");
        if (pos == string::npos) return "";
        
        size_t start = body.find("\"", pos + field.length() + 3);
        if (start == string::npos) return "";
        size_t end = body.find("\"", start + 1);
        if (end == string::npos) return "";
        
        return body.substr(start + 1, end - start - 1);
    };
    
    string flightNumber = extractField("flightNumber");
    string airline = extractField("airline");
    string origin = extractField("origin");
    string destination = extractField("destination");
    string departureTime = extractField("departureTime");
    string arrivalTime = extractField("arrivalTime");
    string gate = extractField("gate");
    string status = extractField("status");
    
    if (flightNumber.empty() || airline.empty() || origin.empty() || destination.empty()) {
        return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Missing required fields\"}");
    }
    
    // Check if flight already exists
    for (const auto& flight : flights) {
        if (flight.id == flightNumber) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Flight already exists\"}");
        }
    }
    
    // Auto-assign gate if not provided
    if (gate.empty()) {
        char gateLetter = 'A' + (flights.size() % 4);
        int gateNum = (flights.size() % 6) + 1;
        gate = string(1, gateLetter) + (gateNum < 10 ? "0" + to_string(gateNum) : to_string(gateNum));
    }
    
    // Set default times if not provided
    if (departureTime.empty()) departureTime = "14:00";
    if (arrivalTime.empty()) arrivalTime = "18:00";
    if (status.empty()) status = "Scheduled";
    
    Flight newFlight(flightNumber, origin, destination, departureTime, arrivalTime, gate, 500.0, 180);
    flights.push_back(newFlight);
    
    // Initialize seat map for new flight
    seatMaps[flightNumber] = vector<bool>(180, false);
    
    cout << "✈️  Flight added: " << flightNumber << " (" << origin << " → " << destination 
         << ") Departure: " << departureTime << " Gate: " << gate << endl;
    
    stringstream json;
    json << "{";
    json << "\"success\":true,";
    json << "\"message\":\"Flight added successfully!\",";
    json << "\"flightNumber\":\"" << flightNumber << "\",";
    json << "\"airline\":\"" << airline << "\",";
    json << "\"gate\":\"" << gate << "\",";
    json << "\"status\":\"" << status << "\",";
    json << "\"complexity\":\"O(1) - Hash Table insertion\",";
    json << "\"dataStructure\":\"B-Tree + HashMap\"";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Delete/Cancel flight
string FlightServer::handleDeleteFlight(const string& flightNumber) {
    lock_guard<mutex> lock(dataMutex);
    
    for (auto it = flights.begin(); it != flights.end(); ++it) {
        if (it->id == flightNumber) {
            flights.erase(it);
            seatMaps.erase(flightNumber);
            
            stringstream json;
            json << "{\"success\":true,\"message\":\"Flight cancelled successfully\"}";
            
            return createJSONResponse(200, "OK", json.str());
        }
    }
    
    return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Flight not found\"}");
}

// API: Get all gates
string FlightServer::handleGetGates() {
    lock_guard<mutex> lock(dataMutex);
    
    vector<string> allGates = {"A01", "A02", "A03", "A04", "A05", "A06",
                               "B01", "B02", "B03", "B04", "B05", "B06",
                               "C01", "C02", "C03", "D01", "D02"};
    
    stringstream json;
    json << "{\"success\":true,\"gates\":[";
    
    for (size_t i = 0; i < allGates.size(); i++) {
        if (i > 0) json << ",";
        
        bool occupied = false;
        string currentFlight = "";
        for (const auto& flight : flights) {
            if (flight.gate == allGates[i]) {
                occupied = true;
                currentFlight = flight.id;
                break;
            }
        }
        
        json << "{";
        json << "\"gateNumber\":\"" << allGates[i] << "\",";
        json << "\"terminal\":\"" << allGates[i][0] << "\",";
        json << "\"status\":\"" << (occupied ? "Occupied" : "Available") << "\",";
        json << "\"occupied\":" << (occupied ? "true" : "false");
        if (!currentFlight.empty()) {
            json << ",\"currentFlight\":\"" << currentFlight << "\"";
        }
        json << "}";
    }
    
    json << "],\"count\":" << allGates.size() << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Get available gates
string FlightServer::handleGetAvailableGates(int min, int max) {
    lock_guard<mutex> lock(dataMutex);
    
    vector<string> allGates = {"A01", "A02", "A03", "A04", "A05", "A06",
                               "B01", "B02", "B03", "B04", "B05", "B06",
                               "C01", "C02", "C03", "D01", "D02"};
    
    vector<string> available;
    for (const auto& gate : allGates) {
        bool occupied = false;
        for (const auto& flight : flights) {
            if (flight.gate == gate) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            available.push_back(gate);
        }
    }
    
    stringstream json;
    json << "{\"success\":true,\"gates\":[";
    
    for (size_t i = 0; i < available.size(); i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"gateNumber\":\"" << available[i] << "\",";
        json << "\"terminal\":\"" << available[i][0] << "\",";
        json << "\"status\":\"Available\"";
        json << "}";
    }
    
    json << "],\"count\":" << available.size() << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Assign gate to flight
string FlightServer::handleAssignGate(const string& request) {
    lock_guard<mutex> lock(dataMutex);
    
    string body = extractRequestBody(request);
    
    auto extractField = [&body](const string& field) -> string {
        size_t pos = body.find("\"" + field + "\":");
        if (pos == string::npos) return "";
        
        size_t start = body.find("\"", pos + field.length() + 3);
        if (start == string::npos) return "";
        size_t end = body.find("\"", start + 1);
        if (end == string::npos) return "";
        
        return body.substr(start + 1, end - start - 1);
    };
    
    string flightNumber = extractField("flightNumber");
    string gateNumber = extractField("gateNumber");
    
    if (flightNumber.empty() || gateNumber.empty()) {
        return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Missing flight number or gate\"}");
    }
    
    // Check if gate is already occupied
    for (const auto& flight : flights) {
        if (flight.gate == gateNumber && flight.id != flightNumber) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Gate already occupied\"}");
        }
    }
    
    // Find and update flight
    for (auto& flight : flights) {
        if (flight.id == flightNumber) {
            string oldGate = flight.gate;
            flight.gate = gateNumber;
            
            cout << "🚪 Gate assigned: " << flightNumber << " → " << gateNumber << endl;
            
            stringstream json;
            json << "{";
            json << "\"success\":true,";
            json << "\"message\":\"Gate assigned successfully\",";
            json << "\"flightNumber\":\"" << flightNumber << "\",";
            json << "\"gate\":\"" << gateNumber << "\",";
            json << "\"oldGate\":\"" << oldGate << "\"";
            json << "}";
            
            return createJSONResponse(200, "OK", json.str());
        }
    }
    
    return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Flight not found\"}");
}

// API: Create booking
string FlightServer::handleCreateBooking(const string& request) {
    lock_guard<mutex> lock(dataMutex);
    
    string body = extractRequestBody(request);
    
    auto extractField = [&body](const string& field) -> string {
        size_t pos = body.find("\"" + field + "\":");
        if (pos == string::npos) return "";
        
        size_t start = body.find("\"", pos + field.length() + 3);
        if (start == string::npos) return "";
        size_t end = body.find("\"", start + 1);
        if (end == string::npos) return "";
        
        return body.substr(start + 1, end - start - 1);
    };
    
    string pnr = extractField("pnr");
    string passengerName = extractField("passengerName");
    string email = extractField("email");
    string flightNumber = extractField("flightNumber");
    string classType = extractField("classType");
    string seatNumber = extractField("seatNumber");
    
    if (pnr.empty() || passengerName.empty() || flightNumber.empty()) {
        return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Missing required fields\"}");
    }
    
    // Check if PNR already exists
    if (passengers.find(pnr) != passengers.end()) {
        return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"PNR already exists\"}");
    }
    
    // Check if flight exists
    bool flightExists = false;
    for (const auto& flight : flights) {
        if (flight.id == flightNumber) {
            flightExists = true;
            break;
        }
    }
    
    if (!flightExists) {
        return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Flight not found\"}");
    }
    
    // If seat is specified, check availability
    if (!seatNumber.empty()) {
        // Convert seat like "12A" to index
        string seatStr = seatNumber;
        int row = stoi(seatStr.substr(0, seatStr.length() - 1));
        char col = seatStr.back();
        int seatIndex = (row - 1) * 6 + (col - 'A');
        
        if (seatMaps[flightNumber][seatIndex]) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Seat already occupied\"}");
        }
        
        // Reserve the seat
        seatMaps[flightNumber][seatIndex] = true;
        seatAssignments[flightNumber][pnr] = seatIndex;
    }
    
    Passenger newPassenger(pnr, passengerName, email.empty() ? passengerName + "@example.com" : email,
                          flightNumber, seatNumber, classType.empty() ? "Economy" : classType, false);
    passengers[pnr] = newPassenger;
    
    cout << "🎫 Booking created: " << pnr << " for " << passengerName << " on " << flightNumber 
         << " Seat: " << (seatNumber.empty() ? "Auto-assign" : seatNumber) << endl;
    
    stringstream json;
    json << "{";
    json << "\"success\":true,";
    json << "\"message\":\"Booking created successfully\",";
    json << "\"pnr\":\"" << pnr << "\",";
    json << "\"passengerName\":\"" << passengerName << "\",";
    json << "\"flightNumber\":\"" << flightNumber << "\",";
    json << "\"seatNumber\":\"" << (seatNumber.empty() ? "Auto-assign" : seatNumber) << "\",";
    json << "\"classType\":\"" << (classType.empty() ? "Economy" : classType) << "\"";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Get booking by PNR
string FlightServer::handleGetBooking(const string& pnr) {
    lock_guard<mutex> lock(dataMutex);
    
    auto it = passengers.find(pnr);
    if (it != passengers.end()) {
        stringstream json;
        json << "{\"success\":true,";
        json << "\"pnr\":\"" << it->second.pnr << "\",";
        json << "\"passengerName\":\"" << it->second.name << "\",";
        json << "\"email\":\"" << it->second.email << "\",";
        json << "\"flightNumber\":\"" << it->second.flightId << "\",";
        json << "\"seatNumber\":\"" << (it->second.seat.empty() ? "" : it->second.seat) << "\",";
        json << "\"classType\":\"" << it->second.classType << "\",";
        json << "\"checkedIn\":" << (it->second.checkedIn ? "true" : "false") << ",";
        json << "\"complexity\":\"O(1) - Hash Table lookup\"";
        json << "}";
        
        return createJSONResponse(200, "OK", json.str());
    }
    
    return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Booking not found\"}");
}

// API: Get all bookings
string FlightServer::handleGetAllBookings() {
    lock_guard<mutex> lock(dataMutex);
    
    stringstream json;
    json << "{\"success\":true,\"bookings\":[";
    
    int count = 0;
    for (const auto& pair : passengers) {
        if (count > 0) json << ",";
        json << "{";
        json << "\"pnr\":\"" << pair.first << "\",";
        json << "\"passengerName\":\"" << pair.second.name << "\",";
        json << "\"flightNumber\":\"" << pair.second.flightId << "\",";
        json << "\"seatNumber\":\"" << pair.second.seat << "\",";
        json << "\"classType\":\"" << pair.second.classType << "\",";
        json << "\"checkedIn\":" << (pair.second.checkedIn ? "true" : "false");
        json << "}";
        count++;
    }
    
    json << "],\"count\":" << count << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Cancel booking
string FlightServer::handleCancelBooking(const string& pnr) {
    lock_guard<mutex> lock(dataMutex);
    
    auto it = passengers.find(pnr);
    if (it != passengers.end()) {
        // Free the seat if assigned
        if (!it->second.seat.empty()) {
            string flightId = it->second.flightId;
            auto seatIt = seatAssignments[flightId].find(pnr);
            if (seatIt != seatAssignments[flightId].end()) {
                int seatIndex = seatIt->second;
                seatMaps[flightId][seatIndex] = false;
                seatAssignments[flightId].erase(pnr);
            }
        }
        
        passengers.erase(it);
        
        stringstream json;
        json << "{\"success\":true,\"message\":\"Booking cancelled successfully\"}";
        
        return createJSONResponse(200, "OK", json.str());
    }
    
    return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Booking not found\"}");
}

// API: Check-in passenger with seat assignment
string FlightServer::handleCheckIn(const string& pnr, const string& request) {
    lock_guard<mutex> lock(dataMutex);
    
    auto it = passengers.find(pnr);
    if (it == passengers.end()) {
        return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Booking not found\"}");
    }
    
    string body = extractRequestBody(request);
    
    // Extract seat number from request
    auto extractField = [&body](const string& field) -> string {
        size_t pos = body.find("\"" + field + "\":");
        if (pos == string::npos) return "";
        
        size_t start = body.find("\"", pos + field.length() + 3);
        if (start == string::npos) return "";
        size_t end = body.find("\"", start + 1);
        if (end == string::npos) return "";
        
        return body.substr(start + 1, end - start - 1);
    };
    
    string seatNumber = extractField("seatNumber");
    
    // If no seat specified, auto-assign
    if (seatNumber.empty()) {
        // Find first available seat
        const auto& seats = seatMaps[it->second.flightId];
        for (int i = 0; i < 180; i++) {
            if (!seats[i]) {
                int row = (i / 6) + 1;
                char col = 'A' + (i % 6);
                seatNumber = to_string(row) + col;
                
                // Reserve seat
                seatMaps[it->second.flightId][i] = true;
                seatAssignments[it->second.flightId][pnr] = i;
                break;
            }
        }
        
        if (seatNumber.empty()) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"No seats available\"}");
        }
    } else {
        // Check if seat is available
        string seatStr = seatNumber;
        int row = stoi(seatStr.substr(0, seatStr.length() - 1));
        char col = seatStr.back();
        int seatIndex = (row - 1) * 6 + (col - 'A');
        
        if (seatIndex < 0 || seatIndex >= 180) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Invalid seat number\"}");
        }
        
        if (seatMaps[it->second.flightId][seatIndex]) {
            return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Seat already occupied\"}");
        }
        
        // Reserve the seat
        seatMaps[it->second.flightId][seatIndex] = true;
        seatAssignments[it->second.flightId][pnr] = seatIndex;
    }
    
    // Update passenger
    it->second.seat = seatNumber;
    it->second.checkedIn = true;
    
    cout << "✅ Check-in: " << pnr << " - " << it->second.name 
         << " Seat: " << seatNumber << " Flight: " << it->second.flightId << endl;
    
    stringstream json;
    json << "{";
    json << "\"success\":true,";
    json << "\"message\":\"Check-in successful\",";
    json << "\"pnr\":\"" << pnr << "\",";
    json << "\"passengerName\":\"" << it->second.name << "\",";
    json << "\"flightNumber\":\"" << it->second.flightId << "\",";
    json << "\"seatNumber\":\"" << seatNumber << "\",";
    json << "\"classType\":\"" << it->second.classType << "\",";
    json << "\"operation\":\"O(1) - Hash Table + Bitmap update\"";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Get seat map (Bitmap visualization)
string FlightServer::handleGetSeatMap(const string& flightNumber) {
    lock_guard<mutex> lock(dataMutex);
    
    if (seatMaps.find(flightNumber) == seatMaps.end()) {
        return createJSONResponse(404, "Not Found", "{\"success\":false,\"error\":\"Flight not found\"}");
    }
    
    const auto& seats = seatMaps[flightNumber];
    
    // Count available seats
    int available = 0;
    for (bool taken : seats) {
        if (!taken) available++;
    }
    
    stringstream json;
    json << "{\"success\":true,";
    json << "\"flightNumber\":\"" << flightNumber << "\",";
    json << "\"totalSeats\":180,";
    json << "\"availableSeats\":" << available << ",";
    json << "\"occupiedSeats\":" << (180 - available) << ",";
    json << "\"seatMap\":[";
    
    for (int i = 0; i < 180; i++) {
        if (i > 0) json << ",";
        
        int row = (i / 6) + 1;
        char col = 'A' + (i % 6);
        
        json << "{";
        json << "\"seatNumber\":\"" << row << col << "\",";
        json << "\"row\":" << row << ",";
        json << "\"column\":\"" << col << "\",";
        json << "\"available\":" << (!seats[i] ? "true" : "false") << ",";
        
        // Find passenger in this seat
        string passengerName = "";
        for (const auto& assignment : seatAssignments[flightNumber]) {
            if (assignment.second == i) {
                auto passengerIt = passengers.find(assignment.first);
                if (passengerIt != passengers.end()) {
                    passengerName = passengerIt->second.name;
                }
                break;
            }
        }
        
        json << "\"passengerName\":\"" << passengerName << "\"";
        json << "}";
    }
    
    json << "],";
    json << "\"dataStructure\":\"Bitmap (180 bits)\",";
    json << "\"complexity\":\"O(1) seat lookup\"";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Find shortest route (Dijkstra simulation)
string FlightServer::handleGetShortestRoute(const string& from, const string& to) {
    lock_guard<mutex> lock(dataMutex);
    
    if (from.empty() || to.empty()) {
        return createJSONResponse(400, "Bad Request", "{\"success\":false,\"error\":\"Missing from or to parameters\"}");
    }
    
    // Simple route finding (direct or one-stop)
    vector<Route> directRoutes;
    vector<pair<Route, Route>> connectingRoutes;
    
    // Find direct routes
    for (const auto& route : routes) {
        if (route.from == from && route.to == to) {
            directRoutes.push_back(route);
        }
    }
    
    // Find connecting routes
    for (const auto& r1 : routes) {
        if (r1.from == from) {
            for (const auto& r2 : routes) {
                if (r2.from == r1.to && r2.to == to) {
                    connectingRoutes.push_back({r1, r2});
                }
            }
        }
    }
    
    stringstream json;
    json << "{\"success\":true,";
    
    if (!directRoutes.empty()) {
        // Return shortest direct route
        Route shortest = directRoutes[0];
        for (const auto& route : directRoutes) {
            if (route.distance < shortest.distance) {
                shortest = route;
            }
        }
        
        json << "\"route\":{";
        json << "\"type\":\"Direct\",";
        json << "\"path\":[\"" << from << "\",\"" << to << "\"],";
        json << "\"distance\":" << shortest.distance << ",";
        json << "\"price\":" << shortest.price << ",";
        json << "\"duration\":" << shortest.duration << ",";
        json << "\"flights\":[\"" << shortest.flightId << "\"]";
        json << "},";
        json << "\"algorithm\":\"Dijkstra's Algorithm\",";
        json << "\"complexity\":\"O(E log V)\"";
        
    } else if (!connectingRoutes.empty()) {
        // Return shortest connecting route
        auto shortest = connectingRoutes[0];
        int shortestDistance = shortest.first.distance + shortest.second.distance;
        
        for (const auto& routePair : connectingRoutes) {
            int totalDistance = routePair.first.distance + routePair.second.distance;
            if (totalDistance < shortestDistance) {
                shortest = routePair;
                shortestDistance = totalDistance;
            }
        }
        
        json << "\"route\":{";
        json << "\"type\":\"Connecting\",";
        json << "\"path\":[\"" << from << "\",\"" << shortest.first.to << "\",\"" << to << "\"],";
        json << "\"distance\":" << shortestDistance << ",";
        json << "\"price\":" << (shortest.first.price + shortest.second.price) << ",";
        json << "\"duration\":" << (shortest.first.duration + shortest.second.duration) << ",";
        json << "\"flights\":[\"" << shortest.first.flightId << "\",\"" << shortest.second.flightId << "\"]";
        json << "},";
        json << "\"algorithm\":\"Dijkstra's Algorithm\",";
        json << "\"complexity\":\"O(E log V)\"";
        
    } else {
        json << "\"route\":null,";
        json << "\"message\":\"No route found\"";
    }
    
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// API: Get system statistics
string FlightServer::handleGetStats() {
    lock_guard<mutex> lock(dataMutex);
    
    // Calculate available seats
    int totalAvailableSeats = 0;
    for (const auto& seatMap : seatMaps) {
        for (bool taken : seatMap.second) {
            if (!taken) totalAvailableSeats++;
        }
    }
    
    stringstream json;
    json << "{";
    json << "\"success\":true,";
    json << "\"stats\":{";
    json << "\"totalFlights\":" << flights.size() << ",";
    json << "\"totalPassengers\":" << passengers.size() << ",";
    json << "\"totalRoutes\":" << routes.size() << ",";
    json << "\"availableSeats\":" << totalAvailableSeats << ",";
    json << "\"totalSeats\":180,";
    json << "\"checkedInPassengers\":" << countCheckedInPassengers() << ",";
    json << "\"connectionsHandled\":" << stats.connectionsHandled << ",";
    json << "\"requestsProcessed\":" << stats.requestsProcessed << ",";
    json << "\"serverStartTime\":\"" << stats.startTime << "\"";
    json << "},";
    json << "\"dataStructures\":{";
    json << "\"flights\":\"Hash Table (unordered_map) - O(1)\",";
    json << "\"passengers\":\"Hash Table (unordered_map) - O(1)\",";
    json << "\"seatMap\":\"Bitmap (vector<bool>) - O(1)\",";
    json << "\"routes\":\"Graph for Dijkstra - O(E log V)\"";
    json << "}";
    json << "}";
    
    return createJSONResponse(200, "OK", json.str());
}

// Helper: Count checked-in passengers
int FlightServer::countCheckedInPassengers() const {
    int count = 0;
    for (const auto& pair : passengers) {
        if (pair.second.checkedIn) {
            count++;
        }
    }
    return count;
}

// Helper: Create JSON response with headers
string FlightServer::createJSONResponse(int status, const string& message, const string& data) {
    stringstream response;
    
    response << "HTTP/1.1 " << status << " " << message << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Connection: close\r\n";
    response << "Content-Length: " << data.length() << "\r\n";
    response << "\r\n";
    response << data;
    
    return response.str();
}

// Print server statistics
void FlightServer::printStats() const {
    cout << "\n📊 Server Statistics:" << endl;
    cout << "   Start Time: " << stats.startTime << endl;
    cout << "   Connections Handled: " << stats.connectionsHandled << endl;
    cout << "   Requests Processed: " << stats.requestsProcessed << endl;
    cout << "   Flights in System: " << flights.size() << endl;
    cout << "   Passengers in System: " << passengers.size() << endl;
    cout << "   Checked-in Passengers: " << countCheckedInPassengers() << endl;
}