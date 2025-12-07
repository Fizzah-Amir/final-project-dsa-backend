#include "gate_functions.h"
#include <algorithm>

GateService::GateService() : gateSchedule("gates_schedule.dat") {
    gateSchedule.initialize();
}

GateService::~GateService() {
    for (auto gate : gates) {
        delete gate;
    }
}

// Add gate
void GateService::addGate(Gate* gate) {
    if (!gate) return;
    
    // Check if exists
    for (auto g : gates) {
        if (g->getId() == gate->getId()) {
            return;
        }
    }
    
    gates.push_back(gate);
}

// Remove gate
void GateService::removeGate(string gateId) {
    for (auto it = gates.begin(); it != gates.end(); ++it) {
        if ((*it)->getId() == gateId) {
            delete *it;
            gates.erase(it);
            return;
        }
    }
}

// Assign best available gate
string GateService::assignBestGate(string flightId, time_t arrival, time_t departure) {
    // Try to find free gate
    for (auto gate : gates) {
        if (gate->isFree()) {
            if (gate->assignFlight(flightId, departure)) {
                // Record in schedule
                gateSchedule.insert(arrival, gate->getId() + ":" + flightId);
                return gate->getId();
            }
        }
    }
    
    return ""; // No gate available
}

// Assign specific gate
bool GateService::assignSpecificGate(string gateId, string flightId, time_t until) {
    Gate* gate = findGate(gateId);
    if (!gate) return false;
    
    if (gate->assignFlight(flightId, until)) {
        // Record in schedule
        gateSchedule.insert(until, gateId + ":" + flightId);
        return true;
    }
    
    return false;
}

// Release gate
bool GateService::releaseGate(string gateId) {
    Gate* gate = findGate(gateId);
    if (!gate) return false;
    
    gate->release();
    return true;
}

// Find gate by ID
Gate* GateService::findGate(string gateId) {
    for (auto gate : gates) {
        if (gate->getId() == gateId) {
            return gate;
        }
    }
    return nullptr;
}

// Get free gates
vector<Gate*> GateService::getFreeGates() {
    vector<Gate*> result;
    
    for (auto gate : gates) {
        if (gate->isFree()) {
            result.push_back(gate);
        }
    }
    
    return result;
}

// Get occupied gates
vector<Gate*> GateService::getOccupiedGates() {
    vector<Gate*> result;
    
    for (auto gate : gates) {
        if (!gate->isFree()) {
            result.push_back(gate);
        }
    }
    
    return result;
}

// Get gates by terminal
vector<Gate*> GateService::getGatesByTerminal(string terminal) {
    vector<Gate*> result;
    
    for (auto gate : gates) {
        if (gate->getTerminal() == terminal) {
            result.push_back(gate);
        }
    }
    
    return result;
}

// Get flights at gate in time range
vector<string> GateService::getFlightsAtGate(string gateId, time_t from, time_t to) {
    vector<string> result;
    
    // Get schedule entries
    auto schedule = gateSchedule.range_query(from, to);
    
    for (auto& pair : schedule) {
        string entry = pair.second;
        size_t colon = entry.find(':');
        
        if (colon != string::npos) {
            string scheduledGate = entry.substr(0, colon);
            string flight = entry.substr(colon + 1);
            
            if (scheduledGate == gateId) {
                result.push_back(flight);
            }
        }
    }
    
    return result;
}

// Get gate for flight at time
string GateService::getGateForFlight(string flightId, time_t atTime) {
    // Check all gates
    for (auto gate : gates) {
        if (gate->getFlight() == flightId) {
            time_t occupied = gate->getOccupiedUntil();
            if (atTime <= occupied) {
                return gate->getId();
            }
        }
    }
    
    return "";
}

// Close gate for maintenance
bool GateService::closeGate(string gateId) {
    Gate* gate = findGate(gateId);
    if (!gate) return false;
    
    gate->closeGate();
    return true;
}

// Open gate
bool GateService::openGate(string gateId) {
    Gate* gate = findGate(gateId);
    if (!gate) return false;
    
    gate->openGate();
    return true;
}

// Count gates
int GateService::countGates() const {
    return gates.size();
}

// Count free gates
int GateService::countFreeGates() const {
    int count = 0;
    for (auto gate : gates) {
        if (gate->isFree()) count++;
    }
    return count;
}

// Count occupied gates
int GateService::countOccupiedGates() const {
    int count = 0;
    for (auto gate : gates) {
        if (!gate->isFree()) count++;
    }
    return count;
}