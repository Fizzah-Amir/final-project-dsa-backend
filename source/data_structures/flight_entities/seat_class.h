// backend/source/data_structures/flight_entities/seat_class.h
#include<string>
#ifndef SEAT_CLASS_H
#define SEAT_CLASS_H

enum SeatClass {
    ECONOMY,
    PREMIUM_ECONOMY,
    BUSINESS,
    FIRST_CLASS
};

enum SeatPosition {
    WINDOW,
    MIDDLE,
    AISLE
};

class Seat {
private:
    int seatNumber;
    SeatClass seatClass;
    SeatPosition position;
    bool isAvailable;
    std::string passengerPNR;
    
public:
    Seat(int number, SeatClass sClass, SeatPosition pos);
    
    // Getters
    int getSeatNumber() const { return seatNumber; }
    SeatClass getSeatClass() const { return seatClass; }
    SeatPosition getPosition() const { return position; }
    bool getIsAvailable() const { return isAvailable; }
    std::string getPassengerPNR() const { return passengerPNR; }
    
    // Setters
    void setAvailable(bool available) { isAvailable = available; }
    void setPassengerPNR(const std::string& pnr) { passengerPNR = pnr; }
    
    // Helper methods
    std::string getClassString() const {
        switch(seatClass) {
            case ECONOMY: return "Economy";
            case PREMIUM_ECONOMY: return "Premium Economy";
            case BUSINESS: return "Business";
            case FIRST_CLASS: return "First Class";
            default: return "Unknown";
        }
    }
    
    std::string getPositionString() const {
        switch(position) {
            case WINDOW: return "Window";
            case MIDDLE: return "Middle";
            case AISLE: return "Aisle";
            default: return "Unknown";
        }
    }
};

#endif