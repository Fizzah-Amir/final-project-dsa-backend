 // backend/source/data_structures/flight_entities/passenger.h
#ifndef PASSENGER_H
#define PASSENGER_H

#include <string>
#include "seat.h"
#include "seat_class.h"  // Include seat.h for SeatClass

class Passenger {
    std::string pnr;
    std::string name;
    std::string contact;
    std::string flightId;
    int seat;
    bool checkedIn;
    SeatClass preferredClass;  // Add this
    std::string seatLetter;    // Add this for seat like "15A"
    
public:
    // Updated constructor
    Passenger(std::string ticket, std::string fullname, 
              std::string phone, std::string flight,
              SeatClass prefClass = ECONOMY);
    
    // Getters
    std::string getPNR() const { return pnr; }
    std::string getName() const { return name; }
    std::string getContact() const { return contact; }
    std::string getFlight() const { return flightId; }
    int getSeat() const { return seat; }
    bool isCheckedIn() const { return checkedIn; }
    SeatClass getPreferredClass() const { return preferredClass; }
    std::string getSeatLetter() const { return seatLetter; }
    
    // Setters
    void setSeat(int s) { seat = s; }
    void setSeatLetter(const std::string& letter) { seatLetter = letter; }
    void setPreferredClass(SeatClass cls) { preferredClass = cls; }
    void checkIn() { checkedIn = true; }
    void cancelCheckIn() { checkedIn = false; }
    
    bool hasSeat() const { return seat > 0; }
    
    std::string getClassString() const {
        switch(preferredClass) {
            case ECONOMY: return "Economy";
            case PREMIUM_ECONOMY: return "Premium Economy";
            case BUSINESS: return "Business";
            case FIRST_CLASS: return "First Class";
            default: return "Unknown";
        }
    }
};

#endif