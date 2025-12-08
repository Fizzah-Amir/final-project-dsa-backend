#include "passenger_functions.h"
#include "../flight_entities/passenger.h"
#include "../flight_entities/seat.h"
#include <iostream>
#include <vector>

using namespace std;

PassengerService::PassengerService() {}

PassengerService::~PassengerService() {
    // Clean up seat maps
    auto all = seatMaps.getAll();
    for (auto& pair : all) {
        delete pair.second;
    }
    
    // Clean up passengers
    auto allPassengers = passengerMap.getAll();
    for (auto& pair : allPassengers) {
        delete pair.second;
    }
}

// FREE SEAT BY PNR - FIXED: Use SeatMap's freeSeatByPNR
bool PassengerService::freeSeatByPNR(string pnr) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    string flightId = passenger->getFlight();
    int seat = passenger->getSeat();
    
    if (seat <= 0) return true; // No seat assigned
    
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) return false;
    
    // Free the seat using SeatMap's method
    seatMap->freeSeatByPNR(pnr);  // This returns void
    
    passenger->setSeat(0);  // Reset seat to 0
    return true;
}

// Add passenger
bool PassengerService::addPassenger(Passenger* passenger) {
    if (!passenger) return false;
    
    string pnr = passenger->getPNR();
    
    // Already exists?
    if (passengerMap.get(pnr)) return false;
    
    passengerMap.insert(pnr, passenger);
    return true;
}

// Find passenger
Passenger* PassengerService::findPassenger(string pnr) {
    return passengerMap.get(pnr);
}

// Remove passenger
bool PassengerService::removePassenger(string pnr) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    // Free seat if assigned
    freeSeatByPNR(pnr);
    
    passengerMap.remove(pnr);
    delete passenger;
    return true;
}

// Check-in
bool PassengerService::checkIn(string pnr) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    passenger->checkIn();
    return true;
}

// Check-in with seat
bool PassengerService::checkInWithSeat(string pnr, int seat) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    // Assign seat
    if (!assignSeat(passenger->getFlight(), seat, pnr)) {
        return false;
    }
    
    passenger->checkIn();
    return true;
}

// Cancel check-in
bool PassengerService::cancelCheckIn(string pnr) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    // Free seat
    freeSeatByPNR(pnr);
    
    passenger->cancelCheckIn();
    return true;
}

// Assign seat - FIXED: Use isAvailable() not isSeatAvailable()
bool PassengerService::assignSeat(string flightId, int seat, string pnr) {
    // Get or create seat map
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) {
        seatMap = new SeatMap(flightId, 180);
        seatMaps.insert(flightId, seatMap);
    }
    
    // Check if seat is already taken - FIXED: Use isAvailable()
    if (!seatMap->isAvailable(seat)) {
        return false;
    }
    
    // Assign seat
    if (!seatMap->takeSeat(seat, pnr)) {
        return false;
    }
    
    // Update passenger
    Passenger* passenger = findPassenger(pnr);
    if (passenger) {
        passenger->setSeat(seat);
    }
    
    return true;
}

// Auto assign seat - FIXED: Use isAvailable() and autoAssign()
int PassengerService::autoAssignSeat(string flightId, string pnr) {
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) {
        seatMap = new SeatMap(flightId, 180);
        seatMaps.insert(flightId, seatMap);
    }
    
    // Use SeatMap's autoAssign method
    int seat = seatMap->autoAssign(pnr);
    if (seat != -1) {
        Passenger* passenger = findPassenger(pnr);
        if (passenger) {
            passenger->setSeat(seat);
        }
    }
    
    return seat;
}

// Change seat
bool PassengerService::changeSeat(string pnr, int newSeat) {
    Passenger* passenger = findPassenger(pnr);
    if (!passenger) return false;
    
    string flight = passenger->getFlight();
    
    // Free current seat
    freeSeatByPNR(pnr);
    
    // Assign new seat
    return assignSeat(flight, newSeat, pnr);
}

// Get seat map display
string PassengerService::getSeatMap(string flightId) {
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) return "No seat map for flight " + flightId;
    return seatMap->showMap();
}

// Get free seats
int PassengerService::getFreeSeats(string flightId) {
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) return 180; // Default capacity
    return seatMap->countFree();
}

// Get booked seats
int PassengerService::getBookedSeats(string flightId) {
    SeatMap* seatMap = seatMaps.get(flightId);
    if (!seatMap) return 0;
    return seatMap->countTaken();
}

// Get all passengers on flight
vector<Passenger*> PassengerService::getPassengersOnFlight(string flightId) {
    vector<Passenger*> result;
    
    auto allPassengers = passengerMap.getAll();
    for (auto& pair : allPassengers) {
        if (pair.second->getFlight() == flightId) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

// Get checked-in passengers
vector<Passenger*> PassengerService::getCheckedInPassengers(string flightId) {
    vector<Passenger*> result;
    auto passengers = getPassengersOnFlight(flightId);
    
    for (auto passenger : passengers) {
        if (passenger->isCheckedIn()) {
            result.push_back(passenger);
        }
    }
    
    return result;
}