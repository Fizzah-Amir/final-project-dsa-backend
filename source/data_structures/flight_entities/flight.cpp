// backend/source/data_structures/flight_entities/flight.cpp
#include "flight.h"
#include <iostream>

// Constructor
Flight::Flight(std::string fid, std::string air, std::string src, 
               std::string dst, std::time_t dep, std::time_t arr)
    : id(fid), airline(air), from(src), to(dst), 
      depart(dep), arrive(arr), gate(""), status(0), 
      seats(180), booked(0) {
}

// Getters
std::string Flight::getFlightId() const { return id; }
std::string Flight::getAirline() const { return airline; }
std::string Flight::getOrigin() const { return from; }
std::string Flight::getDestination() const { return to; }
std::time_t Flight::getDeparture() const { return depart; }
std::time_t Flight::getArrival() const { return arrive; }
std::string Flight::getGate() const { return gate; }
int Flight::getStatus() const { return status; }
int Flight::getTotalSeats() const { return seats; }
int Flight::getBookedSeats() const { return booked; }
int Flight::getFreeSeats() const { return seats - booked; }

// Setters
void Flight::setGate(std::string g) { gate = g; }
void Flight::setStatus(int s) { status = s; }
void Flight::setDeparture(time_t dep) { depart = dep; }
void Flight::setArrival(time_t arr) { arrive = arr; }

// Operations
bool Flight::bookSeat() { 
    if (booked < seats) { 
        booked++; 
        return true; 
    }
    return false;
}

bool Flight::cancelSeat() { 
    if (booked > 0) { 
        booked--; 
        return true; 
    }
    return false;
}

bool Flight::isActive() const { 
    return status != 3; // not cancelled
}

std::string Flight::getStatusText() const {
    switch(status) {
        case 0: return "Scheduled";
        case 1: return "On Time";
        case 2: return "Delayed";
        case 3: return "Cancelled";
        default: return "Unknown";
    }
}