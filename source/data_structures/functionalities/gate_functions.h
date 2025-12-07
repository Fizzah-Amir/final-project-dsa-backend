#ifndef GATESERVICE_H
#define GATESERVICE_H

#include "../data_structures/Btree.h"
#include "../flight_entities/gate.h"
#include <vector>
#include <string>

class GateService {
    std::vector<Gate*> gates;
    BTree gateSchedule; // Time -> Gate mapping
    
public:
    GateService();
    ~GateService();
    
    // Gate setup
    void addGate(Gate* gate);
    void removeGate(std::string gateId);
    
    // Gate allocation
    std::string assignBestGate(std::string flightId, std::time_t arrival, std::time_t departure);
    bool assignSpecificGate(std::string gateId, std::string flightId, std::time_t until);
    bool releaseGate(std::string gateId);
    
    // Queries
    Gate* findGate(std::string gateId);
    std::vector<Gate*> getFreeGates();
    std::vector<Gate*> getOccupiedGates();
    std::vector<Gate*> getGatesByTerminal(std::string terminal);
    
    // Schedule queries
    std::vector<std::string> getFlightsAtGate(std::string gateId, std::time_t from, std::time_t to);
    std::string getGateForFlight(std::string flightId, std::time_t atTime);
    
    // Maintenance
    bool closeGate(std::string gateId);
    bool openGate(std::string gateId);
    
    // Statistics
    int countGates() const;
    int countFreeGates() const;
    int countOccupiedGates() const;
};

#endif