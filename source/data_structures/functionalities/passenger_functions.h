 #ifndef PASSENGERSERVICE_H
#define PASSENGERSERVICE_H

#include "../data_structures/Hashmap.h"
#include "../flight_entities/passenger.h"
#include "../flight_entities/seat.h"
#include <string>
#include <vector>

// Need wrapper structs for value types
struct PassengerWrapper {
    Passenger* ptr;
    PassengerWrapper(Passenger* p = nullptr) : ptr(p) {}
    
    bool operator==(const PassengerWrapper& other) const {
        return ptr == other.ptr;
    }
};

struct SeatMapWrapper {
    SeatMap* ptr;
    SeatMapWrapper(SeatMap* s = nullptr) : ptr(s) {}
    
    bool operator==(const SeatMapWrapper& other) const {
        return ptr == other.ptr;
    }
};

class PassengerService {
    HashMap<std::string, PassengerWrapper> passengerMap; // PNR -> PassengerWrapper
    HashMap<std::string, SeatMapWrapper> seatMaps;       // Flight -> SeatMapWrapper
    
public:
    PassengerService();
    ~PassengerService();
    
    // Passenger operations
    bool addPassenger(Passenger* passenger);
    Passenger* findPassenger(std::string pnr);
    bool removePassenger(std::string pnr);
    
    // Check-in operations
    bool checkIn(std::string pnr);
    bool checkInWithSeat(std::string pnr, int seat);
    bool cancelCheckIn(std::string pnr);
    
    // Seat management
    bool assignSeat(std::string flightId, int seat, std::string pnr);
    int autoAssignSeat(std::string flightId, std::string pnr);
    bool changeSeat(std::string pnr, int newSeat);
    
    // Seat queries
    std::string getSeatMap(std::string flightId);
    int getFreeSeats(std::string flightId);
    int getBookedSeats(std::string flightId);
    
    // Flight-specific queries
    std::vector<Passenger*> getPassengersOnFlight(std::string flightId);
    std::vector<Passenger*> getCheckedInPassengers(std::string flightId);
    
    // ADD MISSING METHOD
    bool freeSeatByPNR(std::string pnr);
};

#endif