 #include "flight_functions.h"
#include <iostream>

FlightService::FlightService() : flightTimeTree("flights.dat") {
    flightTimeTree.initialize();
}

// Add flight
bool FlightService::addFlight(Flight* flight) {
    if (!flight) return false;
    
    string id = flight->getFlightId();
    
    // Already exists?
    Flight** existing = flightMap.get(id);
    if (existing && *existing) return false;
    
    // Add to BTree by time (cast time_t to int)
    flightTimeTree.insert(static_cast<int>(flight->getDeparture()), id);
    
    // Add to HashMap for lookup
    flightMap.insert(id, flight);
    
    return true;
}

// Find flight
Flight* FlightService::findFlight(string id) {
    Flight** flightPtr = flightMap.get(id);
    return flightPtr ? *flightPtr : nullptr;
}

// Remove flight
bool FlightService::removeFlight(string id) {
    Flight* flight = findFlight(id);
    if (!flight) return false;
    
    // Remove from BTree
    flightTimeTree.remove(static_cast<int>(flight->getDeparture()));
    
    // Remove gate assignment
    string gate = getFlightGate(id);
    if (!gate.empty()) {
        freeGate(gate);
    }
    
    // Remove from map
    flightMap.remove(id);
    
    delete flight;
    return true;
}

// Get flights in time range
vector<Flight*> FlightService::getFlightsByTime(time_t from, time_t to) {
    vector<Flight*> result;
    
    // Get IDs from BTree (cast time_t to int)
    auto ids = flightTimeTree.range_query(static_cast<int>(from), static_cast<int>(to));
    
    // Get full flight info
    for (auto& pair : ids) {
        Flight* flight = findFlight(pair.second);
        if (flight) {
            result.push_back(flight);
        }
    }
    
    return result;
}

// Get all flights
vector<Flight*> FlightService::getAllFlights() {
    vector<Flight*> result;
    auto all = flightTimeTree.get_all();
    
    for (auto& pair : all) {
        Flight* flight = findFlight(pair.second);
        if (flight) {
            result.push_back(flight);
        }
    }
    
    return result;
}

// Assign gate
bool FlightService::assignGate(string flightId, string gate) {
    Flight* flight = findFlight(flightId);
    if (!flight) return false;
    
    // Check if gate free
    string* existing = gateMap.get(gate);
    if (existing && *existing != flightId) {
        return false; // Gate taken
    }
    
    // Assign
    gateMap.insert(gate, flightId);
    flight->setGate(gate);
    
    return true;
}

// Get flight's gate
string FlightService::getFlightGate(string flightId) {
    // Search for this flight in gate map
    auto allGates = gateMap.getAll();
    for (auto& pair : allGates) {
        if (pair.second == flightId) {
            return pair.first;
        }
    }
    return "";
}

// Free gate
bool FlightService::freeGate(string gate) {
    string* flightIdPtr = gateMap.get(gate);
    if (flightIdPtr) {
        Flight* flight = findFlight(*flightIdPtr);
        if (flight) {
            flight->setGate("");
        }
    }
    return gateMap.remove(gate);
}

// Delay flight - Now using the setDeparture method we added
bool FlightService::delayFlight(string flightId, int minutes) {
    Flight* flight = findFlight(flightId);
    if (!flight) return false;
    
    // Update departure time
    time_t newTime = flight->getDeparture() + (minutes * 60);
    time_t oldTime = flight->getDeparture();
    
    // Update Flight object using the new setDeparture method
    flight->setDeparture(newTime);
    
    // Update BTree: remove old, insert new
    flightTimeTree.remove(static_cast<int>(oldTime));
    flightTimeTree.insert(static_cast<int>(newTime), flightId);
    
    return true;
}

// Cancel flight
bool FlightService::cancelFlight(string flightId) {
    Flight* flight = findFlight(flightId);
    if (!flight) return false;
    
    flight->setStatus(3); // Cancelled
    return true;
}

// Count flights - Now using the get_flight_count method we added
int FlightService::countFlights() const {
    // Use the new get_flight_count() method from BTree
    return flightTimeTree.get_flight_count();
}

// Count active flights
int FlightService::countActiveFlights() const {
    int count = 0;
    
    // Work around const-correctness issue
    BTree& nonConstTree = const_cast<BTree&>(flightTimeTree);
    auto all = nonConstTree.get_all();
    
    for (auto& pair : all) {
        // Work around HashMap const issue
        Flight** flightPtr = const_cast<HashMap<string, Flight*>&>(flightMap).get(pair.second);
        Flight* flight = flightPtr ? *flightPtr : nullptr;
        if (flight && flight->isActive()) {
            count++;
        }
    }
    
    return count;
}