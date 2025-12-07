#ifndef FLIGHTSERVICE_H
#define FLIGHTSERVICE_H

#include "../data_structures/Btree.h"
#include "../data_structures/Hashmap.h"
#include "../flight_entities/flight.h"
#include <vector>
#include <ctime>

class FlightService {
    BTree flightTimeTree;       // Sorted by departure time
    HashMap<std::string, Flight*> flightMap; // Quick lookup by flight number
    HashMap<std::string, std::string> gateMap; // Flight -> Gate mapping
    
public:
    FlightService();
    
    // Flight operations
    bool addFlight(Flight* flight);
    Flight* findFlight(std::string flightId);
    bool removeFlight(std::string flightId);
    
    // Time-based queries
    std::vector<Flight*> getFlightsByTime(std::time_t from, std::time_t to);
    std::vector<Flight*> getTodayFlights();
    std::vector<Flight*> getAllFlights();
    
    // Gate management
    bool assignGate(std::string flightId, std::string gate);
    std::string getFlightGate(std::string flightId);
    bool freeGate(std::string gate);
    
    // Status updates
    bool delayFlight(std::string flightId, int minutes);
    bool cancelFlight(std::string flightId);
    
    // Statistics
    int countFlights() const;
    int countActiveFlights() const;
};

#endif