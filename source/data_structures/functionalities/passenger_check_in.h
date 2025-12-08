// backend/source/data_structures/functionalities/passenger_check_in.h
#ifndef PASSENGER_CHECK_IN_H
#define PASSENGER_CHECK_IN_H

#include "../Hashmap.h"
#include "../Btree.h"           // Add B-Tree
#include "../flight_entities/passenger.h"
#include "../flight_entities/booking.h"
#include "../flight_entities/seat.h"
#include "../flight_entities/flight.h"    // Add Flight
#include "../flight_entities/gate.h"      // Add Gate
#include "../flight_entities/seat_class.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>

class CheckinService {
private:
    // Existing members
    HashMap<std::string, Passenger*>* passengerMap;
    std::unordered_map<std::string, SeatMap*> seatMaps;
    HashMap<std::string, Booking*>* bookingMap;
    
    // NEW: Flight and Gate management
    BTree* flightSchedule;                          // B-Tree for range queries
    HashMap<std::string, Flight*>* flightMap;       // O(1) flight lookup
    HashMap<std::string, Gate*>* gateMap;           // O(1) gate lookup
    BTree* gateSchedule;                            // B-Tree for gate queries
    
public:
    CheckinService() {
        passengerMap = new HashMap<std::string, Passenger*>(5000);
        bookingMap = new HashMap<std::string, Booking*>(5000);
        flightMap = new HashMap<std::string, Flight*>(1000);
        gateMap = new HashMap<std::string, Gate*>(100);
        
        // Initialize B-Trees
        flightSchedule = new BTree("flight_schedule.dat");
        gateSchedule = new BTree("gate_schedule.dat");
        
        flightSchedule->initialize();
        gateSchedule->initialize();
    }
    
    ~CheckinService() {
        delete passengerMap;
        delete bookingMap;
        delete flightMap;
        delete gateMap;
        
        for (auto& pair : seatMaps) {
            delete pair.second;
        }
        
        delete flightSchedule;
        delete gateSchedule;
    }
    
    // NEW: Add flight to system
    void addFlight(Flight* flight) {
        flightMap->insert(flight->getFlightId(), flight);
        
        // Store in B-Tree by departure time (for range queries)
        int timeKey = static_cast<int>(flight->getDeparture());
        flightSchedule->insert(timeKey, flight->getFlightId());
        
        std::cout << "Added flight " << flight->getFlightId() << " to schedule" << std::endl;
    }
    
    // NEW: Add gate to system
    void addGate(Gate* gate) {
        gateMap->insert(gate->getId(), gate);
        
        // Store in B-Tree by gate number (for finding available gates)
        int gateNumber = std::stoi(gate->getId().substr(1)); // Extract number from "A12"
        gateSchedule->insert(gateNumber, gate->getId());
        
        std::cout << "Added gate " << gate->getId() << " to system" << std::endl;
    }
    
    // NEW: Get flights between times (B-Tree range query)
    std::vector<Flight*> getFlightsBetween(std::time_t start, std::time_t end) {
        std::vector<Flight*> result;
        
        // Use B-Tree range query - O(log n + k)
        auto flightIds = flightSchedule->range_query(
            static_cast<int>(start), 
            static_cast<int>(end)
        );
        
        for (const auto& [timeKey, flightId] : flightIds) {
            Flight** flightPtr = flightMap->get(flightId);
            if (flightPtr) {
                result.push_back(*flightPtr);
            }
        }
        
        std::cout << "Found " << result.size() << " flights between "
                  << std::ctime(&start) << " and " << std::ctime(&end);
        
        return result;
    }
    
    // NEW: Assign gate to flight
    bool assignGateToFlight(const std::string& flightId, const std::string& gateId) {
        Flight** flightPtr = flightMap->get(flightId);
        Gate** gatePtr = gateMap->get(gateId);
        
        if (!flightPtr || !gatePtr) return false;
        
        Flight* flight = *flightPtr;
        Gate* gate = *gatePtr;
        
        if (gate->isFree()) {
            // Calculate occupation time (flight arrival + 30 mins for turnaround)
            std::time_t occupyUntil = flight->getArrival() + (30 * 60);
            
            if (gate->assignFlight(flightId, occupyUntil)) {
                flight->setGate(gateId);
                std::cout << "Assigned gate " << gateId << " to flight " << flightId << std::endl;
                return true;
            }
        }
        
        return false;
    }
    
    // NEW: Find available gates (B-Tree + range query)
    std::vector<Gate*> findAvailableGates(int minGate = 1, int maxGate = 20) {
        std::vector<Gate*> result;
        
        // Get all gates in range from B-Tree
        auto gateIds = gateSchedule->range_query(minGate, maxGate);
        
        for (const auto& [gateNum, gateId] : gateIds) {
            Gate** gatePtr = gateMap->get(gateId);
            if (gatePtr) {
                Gate* gate = *gatePtr;
                if (gate->isFree()) {
                    result.push_back(gate);
                }
            }
        }
        
        std::cout << "Found " << result.size() << " available gates between "
                  << minGate << " and " << maxGate << std::endl;
        
        return result;
    }
    
    // NEW: Get flight status
    std::string getFlightStatus(const std::string& flightId) {
        Flight** flightPtr = flightMap->get(flightId);
        if (flightPtr) {
            Flight* flight = *flightPtr;
            return flight->getStatusText();
        }
        return "Flight not found";
    }
    
    // KEEP ALL EXISTING METHODS - ADD THEM BELOW:
    
    // Add a passenger to the system
    void addPassenger(Passenger* passenger) {
        if (passenger) {
            passengerMap->insert(passenger->getPNR(), passenger);
        }
    }
    
    // Add a booking
    void addBooking(Booking* booking) {
        if (booking) {
            bookingMap->insert(booking->getPNR(), booking);
        }
    }
    
    // Check-in a passenger
    Passenger* checkinPassenger(const std::string& pnr) {
        Passenger** passengerPtr = passengerMap->get(pnr);
        if (!passengerPtr) {
            std::cout << "Passenger not found: " << pnr << std::endl;
            return nullptr;
        }
        
        Passenger* passenger = *passengerPtr;
        
        // Check if already checked in
        if (passenger->isCheckedIn()) {
            std::cout << "Passenger already checked in: " << pnr << std::endl;
            return passenger;
        }
        
        // Mark as checked in
        passenger->checkIn();
        std::cout << "Passenger checked in: " << pnr << std::endl;
        
        // Auto-assign seat if not assigned
        if (!passenger->hasSeat()) {
            SeatMap* seatMap = getSeatMap(passenger->getFlight());
            if (seatMap) {
                int assignedSeat = seatMap->autoAssign(pnr);
                if (assignedSeat > 0) {
                    passenger->setSeat(assignedSeat);
                    std::cout << "Auto-assigned seat " << assignedSeat 
                              << " to passenger " << pnr << std::endl;
                } else {
                    std::cout << "No seats available for auto-assignment" << std::endl;
                }
            }
        }
        
        return passenger;
    }
    
    // Get passenger by PNR
    Passenger* getPassenger(const std::string& pnr) {
        Passenger** passengerPtr = passengerMap->get(pnr);
        return passengerPtr ? *passengerPtr : nullptr;
    }
    
    // Get booking by PNR
    Booking* getBooking(const std::string& pnr) {
        Booking** bookingPtr = bookingMap->get(pnr);
        return bookingPtr ? *bookingPtr : nullptr;
    }
    
    // Assign specific seat to passenger
    bool assignSeat(const std::string& flightNumber, int seatNum, const std::string& pnr) {
        SeatMap* seatMap = getSeatMap(flightNumber);
        if (!seatMap) {
            std::cout << "SeatMap not found for flight: " << flightNumber << std::endl;
            return false;
        }
        
        Passenger** passengerPtr = passengerMap->get(pnr);
        if (!passengerPtr) {
            std::cout << "Passenger not found: " << pnr << std::endl;
            return false;
        }
        
        Passenger* passenger = *passengerPtr;
        
        // Check if seat is available
        if (!seatMap->isAvailable(seatNum)) {
            std::cout << "Seat " << seatNum << " is not available" << std::endl;
            return false;
        }
        
        // Assign seat in SeatMap
        if (seatMap->takeSeat(seatNum, pnr)) {
            // Update passenger seat
            passenger->setSeat(seatNum);
            
            // Update booking if exists
            Booking** bookingPtr = bookingMap->get(pnr);
            if (bookingPtr) {
                Booking* booking = *bookingPtr;
                booking->setSeatNumber(seatNum);
            }
            
            std::cout << "Seat " << seatNum << " assigned to passenger " << pnr << std::endl;
            return true;
        }
        
        return false;
    }
    
    // Get seat map for flight (create if doesn't exist)
    SeatMap* getSeatMap(const std::string& flightNumber) {
        // Create if doesn't exist
        if (seatMaps.find(flightNumber) == seatMaps.end()) {
            seatMaps[flightNumber] = new SeatMap(flightNumber, 180); // Default 180 seats
            std::cout << "Created new SeatMap for flight: " << flightNumber << std::endl;
        }
        return seatMaps[flightNumber];
    }
    
    // Get seat availability visualization
    std::string getSeatMapVisual(const std::string& flightNumber) {
        SeatMap* seatMap = getSeatMap(flightNumber);
        if (seatMap) {
            return seatMap->showMap();
        }
        return "No seat map available for flight: " + flightNumber;
    }
    
    // Cancel booking
    bool cancelBooking(const std::string& pnr) {
        Booking** bookingPtr = bookingMap->get(pnr);
        if (!bookingPtr) {
            std::cout << "Booking not found: " << pnr << std::endl;
            return false;
        }
        
        Booking* booking = *bookingPtr;
        
        // Mark booking as cancelled
        booking->cancel();
        std::cout << "Booking cancelled: " << pnr << std::endl;
        
        // Release seat if assigned
        Passenger** passengerPtr = passengerMap->get(pnr);
        if (passengerPtr) {
            Passenger* passenger = *passengerPtr;
            if (passenger->hasSeat()) {
                SeatMap* seatMap = getSeatMap(passenger->getFlight());
                if (seatMap) {
                    seatMap->freeSeatByPNR(pnr);
                    passenger->setSeat(0); // Reset seat
                    std::cout << "Seat released for passenger: " << pnr << std::endl;
                }
            }
        }
        
        return true;
    }
    
    // Get statistics
    void printStatistics() const {
        std::cout << "\n=== Check-in Service Statistics ===" << std::endl;
        std::cout << "Total passengers: " << passengerMap->getSize() << std::endl;
        std::cout << "Total bookings: " << bookingMap->getSize() << std::endl;
        std::cout << "Total flights with seat maps: " << seatMaps.size() << std::endl;
        std::cout << "Total flights in schedule: " << flightMap->getSize() << std::endl;
        std::cout << "Total gates: " << gateMap->getSize() << std::endl;
        
        for (const auto& pair : seatMaps) {
            SeatMap* seatMap = pair.second;
            std::cout << "\nFlight " << seatMap->getFlight() << ":" << std::endl;
            std::cout << "  Available seats: " << seatMap->countFree() << std::endl;
            std::cout << "  Occupied seats: " << seatMap->countTaken() << std::endl;
        }
    }
};

#endif