// backend/source/data_structures/flight_entities/booking.cpp
#include "booking.h"
#include <iostream>

// Constructor implementation
Booking::Booking(const std::string& pnr, const std::string& flightNum, 
                 const std::string& name, const std::string& contactInfo,
                 SeatClass seatClass, double fareAmount)
    : pnr(pnr), flightNumber(flightNum), passengerName(name), 
      contact(contactInfo), seatNumber(0), bookedClass(seatClass),
      fare(fareAmount), isConfirmed(true), isCancelled(false) {
}

// Setters
void Booking::cancel() { 
    isCancelled = true; 
}

void Booking::setSeatNumber(int seat) { 
    seatNumber = seat; 
}

void Booking::setBookedClass(SeatClass cls) { 
    bookedClass = cls; 
}

void Booking::setFare(double amount) { 
    fare = amount; 
}

void Booking::confirm() { 
    isConfirmed = true; 
}

void Booking::setContact(const std::string& contactInfo) { 
    contact = contactInfo; 
}

// Getters (already inline in header, but could be here if preferred)
std::string Booking::getPNR() const { return pnr; }
std::string Booking::getFlightNumber() const { return flightNumber; }
std::string Booking::getPassengerName() const { return passengerName; }
std::string Booking::getContact() const { return contact; }
int Booking::getSeatNumber() const { return seatNumber; }
SeatClass Booking::getBookedClass() const { return bookedClass; }
double Booking::getFare() const { return fare; }
bool Booking::getIsConfirmed() const { return isConfirmed; }
bool Booking::getIsCancelled() const { return isCancelled; }

// Other methods
bool Booking::isActive() const { 
    return isConfirmed && !isCancelled; 
}

bool Booking::hasSeat() const { 
    return seatNumber > 0; 
}

std::string Booking::getClassString() const {
    switch(bookedClass) {
        case ECONOMY: return "Economy";
        case PREMIUM_ECONOMY: return "Premium Economy";
        case BUSINESS: return "Business";
        case FIRST_CLASS: return "First Class";
        default: return "Unknown";
    }
}