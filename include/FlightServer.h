 #ifndef FLIGHT_SERVER_H
#define FLIGHT_SERVER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>

// Forward declarations
struct Flight;
struct Passenger;
struct Route;

class FlightServer {
private:
    int serverPort;
    int serverSocket;
    bool isRunning;
    
    // Data structures
    std::vector<Flight> flights;
    std::unordered_map<std::string, Passenger> passengers;  // HashMap simulation
    std::vector<Route> routes;
    std::mutex dataMutex;
    
    // Server stats
    struct ServerStats {
        int connectionsHandled;
        int requestsProcessed;
        std::string startTime;
    } stats;
    
    // Helper methods
    std::string extractRequestBody(const std::string& request);
    long long timeToUnix(const std::string& timeStr);
    std::string createJSONResponse(int status, const std::string& message, const std::string& data);
    std::string createJSONResponse(int status, const std::string& jsonData);
    std::string flightToJSON(const Flight& flight);
    std::string passengerToJSON(const Passenger& passenger);
    
    // Request handlers
    std::string handleRequest(const std::string& request);
    void handleClient(int clientSocket);
    
    // API endpoints
    std::string handleHealth();
    std::string handleGetFlights();
    std::string handleSearchFlight(const std::string& flightNumber);
    std::string handleGetFlightsByTime(const std::string& start, const std::string& end);
    std::string handleAddFlight(const std::string& request);
    std::string handleUpdateFlight(const std::string& flightNumber, const std::string& request);
    std::string handleDeleteFlight(const std::string& flightNumber);
    std::string handleGetPassenger(const std::string& pnr);
    std::string handleGetBooking(const std::string& pnr);
    std::string handleGetAllPassengers();
    std::string handleGetAllBookings();
    std::string handleCancelBooking(const std::string& pnr);
    std::string handleCheckIn(const std::string& pnr, const std::string& request);
    std::string handleCreateBooking(const std::string& request);
    std::string handleAssignGate(const std::string& request);
    std::string handleGetGates();
    std::string handleGetAvailableGates(int min, int max);
    std::string handleGetShortestRoute(const std::string& from, const std::string& to);
    std::string handleGetSeatMap(const std::string& flightNumber);
    std::string handleGetStats();
    int countCheckedInPassengers() const;
    
public:
    FlightServer(int port = 8080);
    ~FlightServer();
    
    bool start();
    void stop();
    void printStats() const;
    void initializeData();
};

#endif // FLIGHT_SERVER_H