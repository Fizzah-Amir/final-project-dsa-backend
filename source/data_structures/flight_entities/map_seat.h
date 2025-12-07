// backend/source/data_structures/flight_entities/map_seat.h
#ifndef SEATMAP_H
#define SEATMAP_H

#include "../data_structures/bitmap.h"
#include <string>
#include <unordered_map>

class SeatMap {
    std::string flightId;
    Bitmap* seats;
    int totalSeats;
    
    std::unordered_map<int, std::string> assignments; // seat# -> PNR
    
public:
    SeatMap(std::string fid, int total = 180);
    ~SeatMap();
    
    bool takeSeat(int seat, std::string pnr);
    int autoAssign(std::string pnr);
    void freeSeat(int seat);
    void freeSeatByPNR(std::string pnr);
    
    bool isTaken(int seat) const;
    bool isAvailable(int seat) const { return !isTaken(seat); }
    
    int countFree() const;
    int countTaken() const;
    int getTotalSeats() const { return totalSeats; }
    
    std::string getFlight() const { return flightId; }
    std::string getPassengerAtSeat(int seat) const;
    int getSeatOfPassenger(std::string pnr) const;
    
    std::string showMap() const;
};

#endif