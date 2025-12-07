// backend/source/data_structures/flight_entities/booking.h
#ifndef BOOKING_H
#define BOOKING_H

#include "seat_class.h"
#include <string>

class Booking {
public:
    std::string pnr;
    std::string flightNumber;
    std::string passengerName;
    std::string contact;
    int seatNumber;
    SeatClass bookedClass;
    double fare;
    bool isConfirmed;
    bool isCancelled;
    
    // Constructor declaration
    Booking(const std::string& pnr, const std::string& flightNum, 
            const std::string& name, const std::string& contactInfo,
            SeatClass seatClass = ECONOMY, double fareAmount = 0.0);
    
    // Setters
    void cancel();
    void setSeatNumber(int seat);
    void setBookedClass(SeatClass cls);
    void setFare(double amount);
    void confirm();
    void setContact(const std::string& contactInfo);
    
    // Getters
    std::string getPNR() const;
    std::string getFlightNumber() const;
    std::string getPassengerName() const;
    std::string getContact() const;
    int getSeatNumber() const;
    SeatClass getBookedClass() const;
    double getFare() const;
    bool getIsConfirmed() const;
    bool getIsCancelled() const;
    
    // Other methods
    bool isActive() const;
    bool hasSeat() const;
    std::string getClassString() const;
};

#endif