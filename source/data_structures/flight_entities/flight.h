// backend/source/data_structures/flight_entities/flight.h
#ifndef FLIGHT_H
#define FLIGHT_H

#include <string>
#include <ctime>

class Flight {
    std::string id;           // PK785
    std::string airline;      // PIA
    std::string from;         // ISB
    std::string to;           // DXB
    std::time_t depart;       // Unix time
    std::time_t arrive;
    std::string gate;         // A12
    int status;               // 0=scheduled, 1=on time, 2=delayed, 3=cancelled
    int seats;               // 180
    int booked;              // 120
    
public:
    Flight(std::string fid, std::string air, std::string src, 
           std::string dst, std::time_t dep, std::time_t arr);
    
    // Getters
    std::string getFlightId() const;
    std::string getAirline() const;
    std::string getOrigin() const;
    std::string getDestination() const;
    std::time_t getDeparture() const;
    std::time_t getArrival() const;
    std::string getGate() const;
    int getStatus() const;
    std::string getStatusText() const;
    int getTotalSeats() const;
    int getBookedSeats() const;
    int getFreeSeats() const;
    
    // Setters
    void setGate(std::string g);
    void setStatus(int s);
    void setDeparture(time_t dep);
    void setArrival(time_t arr);
    
    // Operations
    bool bookSeat();
    bool cancelSeat();
    bool isActive() const;
};

#endif