// backend/source/data_structures/flight_entities/gate.cpp
#include "gate.h"
#include <iostream>

Gate::Gate(std::string num, std::string term)
    : number(num), terminal(term), state(0), flight(""), occupied(0) {
}

// ADD THESE GETTER IMPLEMENTATIONS:
std::string Gate::getId() const { 
    return number; 
}

std::string Gate::getTerminal() const { 
    return terminal; 
}

int Gate::getState() const { 
    return state; 
}

std::string Gate::getFlight() const { 
    return flight; 
}

std::time_t Gate::getOccupiedUntil() const { 
    return occupied; 
}

// REST OF THE METHODS (you already have these):
bool Gate::isFree() const {
    return state == 0;
}

bool Gate::assignFlight(std::string fid, std::time_t until) {
    if (isFree()) {
        flight = fid;
        occupied = until;
        state = 1;
        return true;
    }
    return false;
}

void Gate::release() {
    flight = "";
    occupied = 0;
    state = 0;
}

void Gate::closeGate() { 
    state = 2;
}

void Gate::openGate() { 
    state = 0;
}

std::string Gate::getStateText() const {
    switch(state) {
        case 0: return "Free";
        case 1: return "Busy";
        case 2: return "Closed";
        default: return "Unknown";
    }
}