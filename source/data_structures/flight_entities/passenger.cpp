// backend/source/data_structures/flight_entities/passenger.cpp
#include "passenger.h"
#include <iostream>

// Constructor implementation - REQUIRED (not inline in header)
Passenger::Passenger(std::string ticket, std::string fullname, 
                     std::string phone, std::string flight,
                     SeatClass prefClass)
    : pnr(ticket), 
      name(fullname), 
      contact(phone), 
      flightId(flight),
      seat(0), 
      checkedIn(false), 
      preferredClass(prefClass) {
    // Constructor body - you can add initialization logic here
}

// That's it! No other implementations needed