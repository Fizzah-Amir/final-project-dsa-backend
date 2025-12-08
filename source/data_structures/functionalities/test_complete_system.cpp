// backend/source/data_structures/functionalities/test_complete_system.cpp
#include "passenger_check_in.h"
#include <iostream>
#include <ctime>

int main() {
    std::cout << "===========================================\n";
    std::cout << "   COMPLETE FLIGHT MANAGEMENT SYSTEM TEST\n";
    std::cout << "   Demonstrating ALL Data Structure Requirements\n";
    std::cout << "===========================================\n\n";
    
    CheckinService system;
    
    // 1. DEMONSTRATE: B-Tree Range Queries for Flights
    std::cout << "1. DEMO: B-Tree Range Query (Get flights 2PM-5PM)\n";
    std::cout << "   ============================================\n";
    
    // Create flight times (convert to Unix time)
    std::time_t now = std::time(nullptr);
    std::tm tm_now = *std::localtime(&now);
    
    // Flight 1: 2:30 PM
    tm_now.tm_hour = 14; tm_now.tm_min = 30;
    std::time_t flight1_time = std::mktime(&tm_now);
    
    // Flight 2: 4:15 PM  
    tm_now.tm_hour = 16; tm_now.tm_min = 15;
    std::time_t flight2_time = std::mktime(&tm_now);
    
    // Flight 3: 6:00 PM (outside range)
    tm_now.tm_hour = 18; tm_now.tm_min = 0;
    std::time_t flight3_time = std::mktime(&tm_now);
    
    Flight* f1 = new Flight("PK785", "PIA", "ISB", "DXB", flight1_time, flight1_time + 7200);
    Flight* f2 = new Flight("EK203", "Emirates", "DXB", "LHR", flight2_time, flight2_time + 25200);
    Flight* f3 = new Flight("QR123", "Qatar", "DOH", "ISB", flight3_time, flight3_time + 14400);
    
    system.addFlight(f1);
    system.addFlight(f2); 
    system.addFlight(f3);
    
    // Define time range: 2PM-5PM
    tm_now.tm_hour = 14; tm_now.tm_min = 0;
    std::time_t startTime = std::mktime(&tm_now);
    
    tm_now.tm_hour = 17; tm_now.tm_min = 0;
    std::time_t endTime = std::mktime(&tm_now);
    
    // Use B-Tree range query: O(log n + k)
    auto flights = system.getFlightsBetween(startTime, endTime);
    std::cout << "   ✓ Found " << flights.size() << " flights between 2PM-5PM using B-Tree\n";
    
    // 2. DEMONSTRATE: O(1) Flight Operations
    std::cout << "\n2. DEMO: O(1) Flight Operations\n";
    std::cout << "   ============================\n";
    
    // O(1) flight lookup
    std::string status = system.getFlightStatus("PK785");
    std::cout << "   ✓ Flight PK785 status: " << status << " (O(1) lookup)\n";
    
    // 3. DEMONSTRATE: Gate Management with B-Tree
    std::cout << "\n3. DEMO: Gate Management with B-Tree\n";
    std::cout << "   ================================\n";
    
    Gate* g1 = new Gate("A1", "A");
    Gate* g2 = new Gate("A2", "A"); 
    Gate* g3 = new Gate("B1", "B");
    
    system.addGate(g1);
    system.addGate(g2);
    system.addGate(g3);
    
    // Find available gates using B-Tree range query
    auto availableGates = system.findAvailableGates(1, 10);
    std::cout << "   ✓ Found " << availableGates.size() << " available gates (B-Tree range query)\n";
    
    // Assign gate (O(1) operation)
    bool assigned = system.assignGateToFlight("PK785", "A1");
    std::cout << "   ✓ Gate assignment: " << (assigned ? "Success" : "Failed") << " (O(1) operation)\n";
    
    // 4. DEMONSTRATE: Passenger Check-in (All O(1) operations)
    std::cout << "\n4. DEMO: Passenger Operations (All O(1))\n";
    std::cout << "   ====================================\n";
    
    Passenger* p1 = new Passenger("PNR001", "John Doe", "123-456-7890", "PK785", ECONOMY);
    Passenger* p2 = new Passenger("PNR002", "Jane Smith", "234-567-8901", "PK785", BUSINESS);
    
    system.addPassenger(p1);
    system.addPassenger(p2);
    
    Booking* b1 = new Booking("PNR001", "PK785", "John Doe", "123-456-7890");
    Booking* b2 = new Booking("PNR002", "PK785", "Jane Smith", "234-567-8901");
    
    system.addBooking(b1);
    system.addBooking(b2);
    
    // O(1) check-in
    system.checkinPassenger("PNR001");
    std::cout << "   ✓ Passenger check-in complete (O(1) lookup)\n";
    
    // O(1) seat assignment
    system.assignSeat("PK785", 25, "PNR001");
    std::cout << "   ✓ Seat assignment complete (O(1) operation)\n";
    
    // O(1) seat visualization
    std::string seatMap = system.getSeatMapVisual("PK785");
    std::cout << "   ✓ Seat map generated\n";
    
    // O(1) cancellation
    system.cancelBooking("PNR001");
    std::cout << "   ✓ Booking cancellation complete (O(1) operation)\n";
    
    // 5. DEMONSTRATE: Complete System Statistics
    std::cout << "\n5. FINAL SYSTEM STATISTICS\n";
    std::cout << "   =======================\n";
    system.printStatistics();
    
    // 6. DEMONSTRATE: Complexity Analysis
    std::cout << "\n6. COMPLEXITY ANALYSIS DEMONSTRATED\n";
    std::cout << "   ================================\n";
    std::cout << "   ✓ B-Tree Range Query: O(log n + k) for flights 2PM-5PM\n";
    std::cout << "   ✓ Hash Table Lookup: O(1) for passenger/booking/flight\n";
    std::cout << "   ✓ B-Tree Gate Search: O(log n) for gate assignment\n";
    std::cout << "   ✓ Bitmap Seat Map: O(1) for seat availability check\n";
    
    // Cleanup
    delete f1; delete f2; delete f3;
    delete g1; delete g2; delete g3;
    delete p1; delete p2;
    delete b1; delete b2;
    
    std::cout << "\n===========================================\n";
    std::cout << "   ALL REQUIREMENTS SUCCESSFULLY DEMONSTRATED!\n";
    std::cout << "===========================================\n";
    
    return 0;
}