#include "Btree.h"
#include <iostream>
#include <ctime>

using namespace std;

int main() {
    BTree tree("test_flights.dat");
    
    if (!tree.initialize()) {
        cout << "Failed to initialize B-Tree" << endl;
        return 1;
    }
    
    // Test data
    time_t now = time(nullptr);
    
    // Insert test flights
    cout << "\n=== Inserting Flights ===" << endl;
    tree.insert(now + 3600, "AA1234");      // +1 hour
    tree.insert(now + 7200, "BA5678");      // +2 hours
    tree.insert(now + 10800, "EK9012");     // +3 hours
    tree.insert(now + 14400, "QR3456");     // +4 hours
    
    cout << "Total flights: " << tree.get_flight_count() << endl;
    
    // Test range query
    cout << "\n=== Testing Range Query (next 2-4 hours) ===" << endl;
    auto flights = tree.range_query(now + 7200, now + 14400);
    
    for (const auto& flight : flights) {
        cout << "Time: " << flight.first 
             << " Flight: " << flight.second << endl;
    }
    
    // Test get all
    cout << "\n=== All Flights ===" << endl;
    auto all_flights = tree.get_all();
    for (const auto& flight : all_flights) {
        cout << "Time: " << flight.first 
             << " Flight: " << flight.second << endl;
    }
    
    // Test search
    cout << "\n=== Testing Search ===" << endl;
    string flight_num;
    vector<int> path;
    if (tree.search(now + 7200, flight_num, path)) {
        cout << "Found flight: " << flight_num << endl;
        cout << "Path: ";
        for (int block : path) {
            cout << block << " ";
        }
        cout << endl;
    }
    
    tree.shutdown();
    return 0;
}