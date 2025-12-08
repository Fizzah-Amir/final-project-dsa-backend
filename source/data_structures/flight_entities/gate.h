 // backend/source/data_structures/flight_entities/gate.h
#ifndef GATE_H
#define GATE_H

#include <string>
#include <ctime>

class Gate {
    std::string number;     // A12
    std::string terminal;   // A
    int state;             // 0=free, 1=busy, 2=closed
    std::string flight;     // current flight
    std::time_t occupied;   // until when
    
public:
    Gate(std::string num, std::string term);
    
    // Getters
    std::string getId() const;
    std::string getTerminal() const;
    int getState() const;
    std::string getFlight() const;
    std::time_t getOccupiedUntil() const;
    std::string getStateText() const;
    
    // Operations
    bool isFree() const;
    bool assignFlight(std::string fid, std::time_t until);
    void release();
    void closeGate();
    void openGate();
};

#endif